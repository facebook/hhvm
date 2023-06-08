#ifndef fizz_aead_aegis128l_H
#define fizz_aead_aegis128l_H

typedef struct crypto_aead_aegis128l_implementation {
    int (*encrypt_detached)(unsigned char *c, unsigned char *mac, unsigned long long *maclen_p,
                            const unsigned char *m, unsigned long long mlen,
                            const unsigned char *ad, unsigned long long adlen,
                            const unsigned char *nsec, const unsigned char *npub,
                            const unsigned char *k);
    int (*decrypt_detached)(unsigned char *m, unsigned char *nsec, const unsigned char *c,
                            unsigned long long clen, const unsigned char *mac,
                            const unsigned char *ad, unsigned long long adlen,
                            const unsigned char *npub, const unsigned char *k);
} crypto_aead_aegis128l_implementation;

#endif
