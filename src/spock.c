#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ganja.c"

struct spock_state {
    uint32_t Ka[40];
    uint32_t Kb[40];
    uint32_t d[40][4];
};

uint32_t rotl(uint32_t a, int b) {
    return ((a << b) | (a >> (32 - b)));
}

uint32_t rotr(uint32_t a, int b) {
    return ((a >> b) | (a << (32 - b)));
}

void roundF(struct spock_state *state, uint32_t *xla, uint32_t *xlb, uint32_t *xra, uint32_t *xrb, int rounds) {
    uint32_t a, b, c, d;
    a = *xla;
    b = *xlb;
    c = *xra;
    d = *xrb;
    for (int r = 0; r < rounds; r++) {
        a = rotr(a, 8);
	a += d;
        a ^= state->Ka[r];
        b = rotr(b, 7);
	b += c;
        b ^= state->Kb[r];
	c = rotl(c, 2);
	c ^= b;
	d = rotl(d, 3);
	d ^= a;
	a += b;
	b += a;
	a += state->d[r][0];
	b += state->d[r][1];
	c += state->d[r][2];
	d += state->d[r][3];
    }
    *xla = a;
    *xlb = b;
    *xra = c;
    *xrb = d;
}

void roundB(struct spock_state *state, uint32_t *xla, uint32_t *xlb, uint32_t *xra, uint32_t *xrb, int rounds) {
    uint32_t a, b, c, d;
    a = *xla;
    b = *xlb;
    c = *xra;
    d = *xrb;
    for (int r = rounds; r --> 0;) {
	d -= state->d[r][3];
	c -= state->d[r][2];
	b -= state->d[r][1];
	a -= state->d[r][0];
	b -= a;
	a -= b;
	d ^= a;
	d = rotr(d, 3);
	c ^= b;
	c = rotr(c, 2);
        b ^= state->Kb[r];
	b -= c;
        b = rotl(b, 7);
        a ^= state->Ka[r];
	a -= d;
        a = rotl(a, 8);
    }
    *xla = a;
    *xlb = b;
    *xra = c;
    *xrb = d;
}

void spock_ksa(struct spock_state *state, unsigned char * key, int keylen, int rounds) {
    uint32_t temp = 0x00000001;
    struct spock_state tempstate;
    int m = 0;
    int b;
    int inc = keylen / 4;
    int step = inc / 4;
    uint32_t *k[inc];
    for (int i = 0; i < inc; i++) {
        k[i] = 0;
        k[i] = (key[m] << 24) + (key[m+1] << 16) + (key[m+2] << 8) + key[m+3];
        m += step;
    }

    int c = 0;
    for (int r = 0; r < (rounds / inc); r++) {
        for (int i = 0; i < inc; i++) {
            tempstate.Ka[c] = k[i];
            tempstate.Kb[c] = k[i];
            c += 1;
        }
    }
    c = 0;
    for (int r = 0; r < rounds; r++) {
        for (int i = 0; i < 4; i++) {
            state->d[r][i] = 0;
            tempstate.d[r][i] = k[i];
        }
    }
    c = 0;
    b = 0;
    for (int r = 0; r < (rounds / inc); r++) {
        m = 0;
        for (int i = 0; i < (inc / 4); i++) {
            roundF(&tempstate, &k[m], &k[m+1], &k[m+2], &k[m+3], rounds);
            m += 4;
        }
        for (int i = 0; i < inc; i++) {
            state->Ka[c] = k[i];
            c += 1;
        }
        m = 0;
        for (int i = 0; i < (inc / 4); i++) {
            roundF(&tempstate, &k[m], &k[m+1], &k[m+2], &k[m+3], rounds);
            m += 4;
        }
        for (int i = 0; i < inc; i++) {
            state->Kb[b] = k[i];
            b += 1;
        }
    }
    for (int r = 0; r < rounds; r++) {
        m = 0;
        for (int i = 0; i < (inc / 4); i++) {
            roundF(&tempstate, &k[m], &k[m+1], &k[m+2], &k[m+3], rounds);
            m += 4;
        }
        state->d[r][0] = k[0];
        state->d[r][1] = k[1];
        state->d[r][2] = k[2];
        state->d[r][3] = k[3];
    }
}
