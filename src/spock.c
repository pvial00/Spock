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

void ksa(struct spock_state *state, unsigned char * key) {
    uint32_t *k[4];
    uint32_t temp = 0x00000001;
    struct spock_state tempstate;
    k[0] = (key[0] << 24) + (key[1] << 16) + (key[2] << 8) + key[3];
    k[1] = (key[4] << 24) + (key[5] << 16) + (key[6] << 8) + key[7];
    k[2] = (key[8] << 24) + (key[9] << 16) + (key[10] << 8) + key[11];
    k[3] = (key[12] << 24) + (key[13] << 16) + (key[14] << 8) + key[15];
    
    int c = 0;
    for (int r = 0; r < (40 / 4); r++) {
        tempstate.Ka[c] = k[0];
        tempstate.Ka[c+1] = k[1];
        tempstate.Ka[c+2] = k[2];
        tempstate.Ka[c+3] = k[3];
        tempstate.Kb[c] = k[0];
        tempstate.Kb[c+1] = k[1];
        tempstate.Kb[c+2] = k[2];
        tempstate.Kb[c+3] = k[3];
	c += 4;
    }
    c = 0;
    for (int r = 0; r < 40; r++) {
        for (int i = 0; i < 4; i++) {
            state->d[r][i] = 0;
	    tempstate.d[r][i] = k[i];
        }
    }
    c = 0;
    for (int r = 0; r < (40 / 4); r++) {
	roundF(&tempstate, &k[0], &k[1], &k[2], &k[3], 40);
        state->Ka[c] = k[0];
        state->Ka[c+1] = k[1];
        state->Ka[c+2] = k[2];
        state->Ka[c+3] = k[3];
	roundF(&tempstate, &k[0], &k[1], &k[2], &k[3], 40);
        state->Kb[c] = k[0];
        state->Kb[c+1] = k[1];
        state->Kb[c+2] = k[2];
        state->Kb[c+3] = k[3];
	c += 4;
    }
    for (int r = 0; r < 40; r++) {
	roundF(&tempstate, &k[0], &k[1], &k[2], &k[3], 40);
        state->d[r][0] = k[0];
        state->d[r][1] = k[1];
        state->d[r][2] = k[2];
        state->d[r][3] = k[3];
    }
}
