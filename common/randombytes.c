/*
The MIT License

Copyright (c) 2017 Daan Sprenkels <hello@dsprenkels.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifdef RANDOM_RANDOMBYTES

// In the case that are compiling on linux, we need to define _GNU_SOURCE
// *before* randombytes.h is included. Otherwise SYS_getrandom will not be
// declared.
#if defined(__linux__)
#define _GNU_SOURCE
#endif /* defined(__linux__) */

#include "randombytes.h"

#if defined(_WIN32)
/* Windows */
// NOLINTNEXTLINE(llvm-include-order): Include order required by Windows
#include <windows.h>
#include <wincrypt.h> /* CryptAcquireContext, CryptGenRandom */
#endif /* defined(_WIN32) */

#if defined(__linux__)
/* Linux */
// We would need to include <linux/random.h>, but not every target has access
// to the linux headers. We only need RNDGETENTCNT, so we instead inline it.
// RNDGETENTCNT is originally defined in `include/uapi/linux/random.h` in the
// linux repo.
#define RNDGETENTCNT 0x80045200

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

// We need SSIZE_MAX as the maximum read len from /dev/urandom
#if !defined(SSIZE_MAX)
#define SSIZE_MAX (SIZE_MAX / 2 - 1)
#endif /* defined(SSIZE_MAX) */

#endif /* defined(__linux__) */

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
/* Dragonfly, FreeBSD, NetBSD, OpenBSD (has arc4random) */
#include <sys/param.h>
#if defined(BSD)
#include <stdlib.h>
#endif
#endif

#if defined(__EMSCRIPTEN__)
#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <stdbool.h>
#endif /* defined(__EMSCRIPTEN__) */

#if defined(_WIN32)
static int randombytes_win32_randombytes(void *buf, const size_t n) {
    HCRYPTPROV ctx;
    BOOL tmp;

    tmp = CryptAcquireContext(&ctx, NULL, NULL, PROV_RSA_FULL,
                              CRYPT_VERIFYCONTEXT);
    if (tmp == FALSE) {
        return -1;
    }

    tmp = CryptGenRandom(ctx, (DWORD)n, (BYTE *)buf);
    if (tmp == FALSE) {
        return -1;
    }

    tmp = CryptReleaseContext(ctx, 0);
    if (tmp == FALSE) {
        return -1;
    }

    return 0;
}
#endif /* defined(_WIN32) */

#if defined(__linux__) && defined(SYS_getrandom)
static int randombytes_linux_randombytes_getrandom(void *buf, size_t n) {
    /* I have thought about using a separate PRF, seeded by getrandom, but
     * it turns out that the performance of getrandom is good enough
     * (250 MB/s on my laptop).
     */
    size_t offset = 0, chunk;
    long int ret;
    while (n > 0) {
        /* getrandom does not allow chunks larger than 33554431 */
        chunk = n <= 33554431 ? n : 33554431;
        do {
            ret = syscall(SYS_getrandom, (char *)buf + offset, chunk, 0);
        } while (ret == -1 && errno == EINTR);
        if (ret < 0) {
            return (int) ret;
        }
        offset += (size_t) ret;
        n -= (size_t) ret;
    }
    assert(n == 0);
    return 0;
}
#endif /* defined(__linux__) && defined(SYS_getrandom) */

#if defined(__linux__) && !defined(SYS_getrandom)
static int randombytes_linux_read_entropy_ioctl(int device, int *entropy) {
    return ioctl(device, RNDGETENTCNT, entropy);
}

static int randombytes_linux_read_entropy_proc(FILE *stream, int *entropy) {
    int retcode;
    do {
        rewind(stream);
        retcode = fscanf(stream, "%d", entropy);
    } while (retcode != 1 && errno == EINTR);
    if (retcode != 1) {
        return -1;
    }
    return 0;
}

static int randombytes_linux_wait_for_entropy(int device) {
    /* We will block on /dev/random, because any increase in the OS' entropy
     * level will unblock the request. I use poll here (as does libsodium),
     * because we don't *actually* want to read from the device. */
    enum { IOCTL,
           PROC
         } strategy = IOCTL;
    const int bits = 128;
    struct pollfd pfd;
    int fd;
    FILE *proc_file;
    int retcode,
        retcode_error = 0; // Used as return codes throughout this function
    int entropy = 0;

    /* If the device has enough entropy already, we will want to return early */
    retcode = randombytes_linux_read_entropy_ioctl(device, &entropy);
    if (retcode != 0 && errno == ENOTTY) {
        /* The ioctl call on /dev/urandom has failed due to a ENOTTY (i.e.
         * unsupported action). We will fall back to reading from
         * `/proc/sys/kernel/random/entropy_avail`. This is obviously less
         * ideal, but at this point it seems we have no better option. */
        strategy = PROC;
        // Open the entropy count file
        proc_file = fopen("/proc/sys/kernel/random/entropy_avail", "r");
    } else if (retcode != 0) {
        // Unrecoverable ioctl error
        return -1;
    }
    if (entropy >= bits) {
        return 0;
    }

    do {
        fd = open("/dev/random", O_RDONLY);
    } while (fd == -1 && errno == EINTR); /* EAGAIN will not occur */
    if (fd == -1) {
        /* Unrecoverable IO error */
        return -1;
    }

    pfd.fd = fd;
    pfd.events = POLLIN;
    for (;;) {
        retcode = poll(&pfd, 1, -1);
        if (retcode == -1 && (errno == EINTR || errno == EAGAIN)) {
            continue;
        } else if (retcode == 1) {
            if (strategy == IOCTL) {
                retcode =
                    randombytes_linux_read_entropy_ioctl(device, &entropy);
            } else if (strategy == PROC) {
                retcode =
                    randombytes_linux_read_entropy_proc(proc_file, &entropy);
            } else {
                return -1; // Unreachable
            }

            if (retcode != 0) {
                // Unrecoverable I/O error
                retcode_error = retcode;
                break;
            }
            if (entropy >= bits) {
                break;
            }
        } else {
            // Unreachable: poll() should only return -1 or 1
            retcode_error = -1;
            break;
        }
    }
    do {
        retcode = close(fd);
    } while (retcode == -1 && errno == EINTR);
    if (strategy == PROC) {
        do {
            retcode = fclose(proc_file);
        } while (retcode == -1 && errno == EINTR);
    }
    if (retcode_error != 0) {
        return retcode_error;
    }
    return retcode;
}

static int randombytes_linux_randombytes_urandom(void *buf, size_t n) {
    int fd;
    size_t offset = 0, count;
    ssize_t tmp;
    do {
        fd = open("/dev/urandom", O_RDONLY);
    } while (fd == -1 && errno == EINTR);
    if (fd == -1) {
        return -1;
    }
    if (randombytes_linux_wait_for_entropy(fd) == -1) {
        return -1;
    }

    while (n > 0) {
        count = n <= SSIZE_MAX ? n : SSIZE_MAX;
        tmp = read(fd, (char *)buf + offset, count);
        if (tmp == -1 && (errno == EAGAIN || errno == EINTR)) {
            continue;
        }
        if (tmp == -1) {
            return -1; /* Unrecoverable IO error */
        }
        offset += tmp;
        n -= tmp;
    }
    assert(n == 0);
    return 0;
}
#endif /* defined(__linux__) && !defined(SYS_getrandom) */

#if defined(BSD)
static int randombytes_bsd_randombytes(void *buf, size_t n) {
    arc4random_buf(buf, n);
    return 0;
}
#endif /* defined(BSD) */

#if defined(__EMSCRIPTEN__)
static int randombytes_js_randombytes_nodejs(void *buf, size_t n) {
    const int ret = EM_ASM_INT({
        var crypto;
        try {
            crypto = require('crypto');
        } catch (error) {
            return -2;
        }
        try {
            writeArrayToMemory(crypto.randomBytes($1), $0);
            return 0;
        } catch (error) {
            return -1;
        }
    },
    buf, n);
    switch (ret) {
    case 0:
        return 0;
    case -1:
        errno = EINVAL;
        return -1;
    case -2:
        errno = ENOSYS;
        return -1;
    }
    assert(false); // Unreachable
}
#endif /* defined(__EMSCRIPTEN__) */

int randombytes(uint8_t *buf, size_t n) {
    #if defined(__EMSCRIPTEN__)
    return randombytes_js_randombytes_nodejs(buf, n);
    #elif defined(__linux__)
    #if defined(SYS_getrandom)
    /* Use getrandom system call */
    return randombytes_linux_randombytes_getrandom(buf, n);
    #else
    /* When we have enough entropy, we can read from /dev/urandom */
    return randombytes_linux_randombytes_urandom(buf, n);
    #endif
    #elif defined(BSD)
    /* Use arc4random system call */
    return randombytes_bsd_randombytes(buf, n);
    #elif defined(_WIN32)
    /* Use windows API */
    return randombytes_win32_randombytes(buf, n);
    #else
#error "randombytes(...) is not supported on this platform"
    #endif
}

#else

#include <assert.h>
#include <string.h>

#include "aes.h"
#include "randombytes.h"

typedef struct {
    uint8_t Key[32];
    uint8_t V[16];
    int reseed_counter;
} AES256_CTR_DRBG_struct;

static AES256_CTR_DRBG_struct DRBG_ctx;
static void AES256_CTR_DRBG_Update(const uint8_t *provided_data, uint8_t *Key, uint8_t *V);

// Use whatever AES implementation you have. This uses AES from openSSL library
//    key - 256-bit AES key
//    ctr - a 128-bit plaintext value
//    buffer - a 128-bit ciphertext value
static void AES256_ECB(uint8_t *key, uint8_t *ctr, uint8_t *buffer) {
    aes256ctx ctx;
    aes256_ecb_keyexp(&ctx, key);
    aes256_ecb(buffer, ctr, 1, &ctx);
    aes256_ctx_release(&ctx);
}

void nist_kat_init(uint8_t *entropy_input, const uint8_t *personalization_string, int security_strength);
void nist_kat_init(uint8_t *entropy_input, const uint8_t *personalization_string, int security_strength) {
    uint8_t seed_material[48];

    assert(security_strength == 256);
    memcpy(seed_material, entropy_input, 48);
    if (personalization_string) {
        for (int i = 0; i < 48; i++) {
            seed_material[i] ^= personalization_string[i];
        }
    }
    memset(DRBG_ctx.Key, 0x00, 32);
    memset(DRBG_ctx.V, 0x00, 16);
    AES256_CTR_DRBG_Update(seed_material, DRBG_ctx.Key, DRBG_ctx.V);
    DRBG_ctx.reseed_counter = 1;
}

int randombytes(uint8_t *buf, size_t n) {
    uint8_t block[16];
    int i = 0;

    while (n > 0) {
        //increment V
        for (int j = 15; j >= 0; j--) {
            if (DRBG_ctx.V[j] == 0xff) {
                DRBG_ctx.V[j] = 0x00;
            } else {
                DRBG_ctx.V[j]++;
                break;
            }
        }
        AES256_ECB(DRBG_ctx.Key, DRBG_ctx.V, block);
        if (n > 15) {
            memcpy(buf + i, block, 16);
            i += 16;
            n -= 16;
        } else {
            memcpy(buf + i, block, n);
            n = 0;
        }
    }
    AES256_CTR_DRBG_Update(NULL, DRBG_ctx.Key, DRBG_ctx.V);
    DRBG_ctx.reseed_counter++;
    return 0;
}

static void AES256_CTR_DRBG_Update(const uint8_t *provided_data, uint8_t *Key, uint8_t *V) {
    uint8_t temp[48];

    for (int i = 0; i < 3; i++) {
        //increment V
        for (int j = 15; j >= 0; j--) {
            if (V[j] == 0xff) {
                V[j] = 0x00;
            } else {
                V[j]++;
                break;
            }
        }

        AES256_ECB(Key, V, temp + 16 * i);
    }
    if (provided_data != NULL) {
        for (int i = 0; i < 48; i++) {
            temp[i] ^= provided_data[i];
        }
    }
    memcpy(Key, temp, 32);
    memcpy(V, temp + 32, 16);
}

#endif


