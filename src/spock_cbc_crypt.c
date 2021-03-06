#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ganja.c"
#include "dyefamily.c"

struct spock_state {
    uint32_t Ka[48];
    uint32_t Kb[48];
    uint32_t d[48][4];
    int rounds;
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

void spock_ksa(struct spock_state *state, unsigned char * keyp, int keylen) {
    uint32_t temp = 0x00000001;
    struct spock_state tempstate;
    int m = 0;
    int b, i, r, x;
    uint32_t k[8];
    memset(k, 0, 8*sizeof(uint32_t));
    memset(state->Ka, 0, state->rounds*sizeof(uint32_t));
    memset(state->Kb, 0, state->rounds*sizeof(uint32_t));
    memset(tempstate.Ka, 0, state->rounds*sizeof(uint32_t));
    memset(tempstate.Kb, 0, state->rounds*sizeof(uint32_t));
    memset(state->d, 0, 4*(state->rounds*sizeof(uint32_t)));
    memset(tempstate.d, 0, 4*(state->rounds*sizeof(uint32_t)));
    for (i = 0; i < 8; i++) {
        k[i] = 0;
        k[i] = (keyp[m] << 24) + (keyp[m+1] << 16) + (keyp[m+2] << 8) + keyp[m+3];
        m += 4;
    }

    for (r = 0; r < state->rounds; r++) {
        k[0] = rotr(k[0], 8);
        k[0] += k[4];
        k[0] ^= k[6];
        k[1] = rotr(k[1], 7);
        k[1] += k[2];
        k[1] ^= k[0];
        k[2] = rotr(k[2], 2);
        k[2] ^= k[1];
        k[3] = rotl(k[3], 3);
        k[3] ^= k[5];
        k[3] += k[7];

        k[4] = rotr(k[5], 8);
        k[4] += k[3];
        k[4] ^= k[2];
        k[5] = rotr(k[5], 7);
        k[5] += k[0];
        k[5] ^= k[6];
        k[6] = rotr(k[6], 2);
        k[6] ^= k[2];
        k[7] = rotl(k[7], 3);
        k[7] ^= k[4];
        k[7] += k[5];
        for (i = 0; i < 8; i++) {
            tempstate.Ka[r] ^= (uint32_t)k[i];
        }
    }
    for (r = 0; r < state->rounds; r++) {
        k[0] = rotr(k[0], 8);
        k[0] += k[4];
        k[0] ^= k[6];
        k[1] = rotr(k[1], 7);
        k[1] += k[2];
        k[1] ^= k[0];
        k[2] = rotr(k[2], 2);
        k[2] ^= k[1];
        k[3] = rotl(k[3], 3);
        k[3] ^= k[5];
        k[3] += k[7];

        k[4] = rotr(k[5], 8);
        k[4] += k[3];
        k[4] ^= k[2];
        k[5] = rotr(k[5], 7);
        k[5] += k[0];
        k[5] ^= k[6];
        k[6] = rotr(k[6], 2);
        k[6] ^= k[2];
        k[7] = rotl(k[7], 3);
        k[7] ^= k[4];
        k[7] += k[5];
        for (i = 0; i < 8; i++) {
            tempstate.Kb[r] ^= (uint32_t)k[i];
        }
    }
    for (r = 0; r < state->rounds; r++) {
        for (i = 0; i < 4; i++) {
            k[0] = rotr(k[0], 8);
            k[0] += k[4];
            k[0] ^= k[6];
            k[1] = rotr(k[1], 7);
            k[1] += k[2];
            k[1] ^= k[0];
            k[2] = rotr(k[2], 2);
            k[2] ^= k[1];
            k[3] = rotl(k[3], 3);
            k[3] ^= k[5];
            k[3] += k[7];

            k[4] = rotr(k[5], 8);
            k[4] += k[3];
            k[4] ^= k[2];
            k[5] = rotr(k[5], 7);
            k[5] += k[0];
            k[5] ^= k[6];
            k[6] = rotr(k[6], 2);
            k[6] ^= k[2];
            k[7] = rotl(k[7], 3);
            k[7] ^= k[4];
            k[7] += k[5];
            for (x = 0; x < 8; x++) {
                tempstate.d[r][i] ^= (uint32_t)k[x];
            }
        }
    }
    for (r = 0; r < state->rounds; r++) {
        k[0] = rotr(k[0], 8);
        k[0] += k[4];
        k[0] ^= k[6];
        k[1] = rotr(k[1], 7);
        k[1] += k[2];
        k[1] ^= k[0];
        k[2] = rotr(k[2], 2);
        k[2] ^= k[1];
        k[3] = rotl(k[3], 3);
        k[3] ^= k[5];
        k[3] += k[7];

        k[4] = rotr(k[5], 8);
        k[4] += k[3];
        k[4] ^= k[2];
        k[5] = rotr(k[5], 7);
        k[5] += k[0];
        k[5] ^= k[6];
        k[6] = rotr(k[6], 2);
        k[6] ^= k[2];
        k[7] = rotl(k[7], 3);
        k[7] ^= k[4];
        k[7] += k[5];
        for (i = 0; i < 8; i++) {
            state->Ka[r] ^= (uint32_t)k[i];
        }
        k[0] = rotr(k[0], 8);
        k[0] += k[4];
        k[0] ^= k[6];
        k[1] = rotr(k[1], 7);
        k[1] += k[2];
        k[1] ^= k[0];
        k[2] = rotr(k[2], 2);
        k[2] ^= k[1];
        k[3] = rotl(k[3], 3);
        k[3] ^= k[5];
        k[3] += k[7];

        k[4] = rotr(k[5], 8);
        k[4] += k[3];
        k[4] ^= k[2];
        k[5] = rotr(k[5], 7);
        k[5] += k[0];
        k[5] ^= k[6];
        k[6] = rotr(k[6], 2);
        k[6] ^= k[2];
        k[7] = rotl(k[7], 3);
        k[7] ^= k[4];
        k[7] += k[5];
        for (i = 0; i < 8; i++) {
            state->Kb[r] ^= (uint32_t)k[i];
        }
    }
    for (r = 0; r < state->rounds; r++) {
        for (i = 0; i < 4; i++) {
            k[0] = rotr(k[0], 8);
            k[0] += k[4];
            k[0] ^= k[6];
            k[1] = rotr(k[1], 7);
            k[1] += k[2];
            k[1] ^= k[0];
            k[2] = rotr(k[2], 2);
            k[2] ^= k[1];
            k[3] = rotl(k[3], 3);
            k[3] ^= k[5];
            k[3] += k[7];

            k[4] = rotr(k[5], 8);
            k[4] += k[3];
            k[4] ^= k[2];
            k[5] = rotr(k[5], 7);
            k[5] += k[0];
            k[5] ^= k[6];
            k[6] = rotr(k[6], 2);
            k[6] ^= k[2];
            k[7] = rotl(k[7], 3);
            k[7] ^= k[4];
            k[7] += k[5];
            for (x = 0; x < 8; x++) {
                state->d[r][i] ^= (uint32_t)k[x];
            }
        }
    }
}

int main(int arc, char *argv[]) {
    unsigned char * key[16] = {0};
    FILE *outfile, *infile;
    uint8_t k[16];
    uint32_t block[4];
    uint32_t last[4];
    uint32_t next[4];
    struct spock_state state;
    int iv_length = 16;
    int keylen = 16;
    state.rounds = 48;
    unsigned char iv[iv_length];
    int c = 0;
    char *mode = argv[1];
    char *inf = argv[2];
    char *outf = argv[3];
    char *password = argv[4];
    ganja_kdf(password, strlen(password), key, 10000, keylen, "WildLFSRCipherv1");
    spock_ksa(&state, key, keylen);
    int v = 16;
    int x, i;
    int t = 0;
    infile = fopen(inf, "rb");
    fseek(infile, 0, SEEK_END);
    long fsize = ftell(infile);
    fseek(infile, 0, SEEK_SET);
    unsigned char data[v];
    outfile = fopen(outf, "wb");
    if (strcmp(mode, "e") == 0) {
        int blocks = fsize / 16;
        int fsize_extra = fsize % 16;
        int extra = 0;
        if (fsize_extra != 0) {
            blocks += 1;
        }
        wrzeszcz_random(&iv, iv_length);
        fwrite(iv, 1, iv_length, outfile);
        for (int i = 0; i < 4; i++) {
            last[i] = (iv[c] << 24) + (iv[c+1] << 16) + (iv[c+2] << 8) + iv[c+3];
            c += 4;
        }
        for (i = 0; i < (blocks); i++) {
            fread(data, 1, v, infile);
            if (i == (blocks - 1)) {
                int g = 15;
                for (int b = 0; b < (v - fsize_extra); b++) {
                    data[g] = (v - fsize_extra);
		    g = (g - 1);
                }
            }
            block[0] = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
            block[1] = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + data[7];
            block[2] = (data[8] << 24) + (data[9] << 16) + (data[10] << 8) + data[11];
            block[3] = (data[12] << 24) + (data[13] << 16) + (data[14] << 8) + data[15];
            for (int r = 0; r < 4; r++) {
                block[r] = block[r] ^ last[r];
            }
	    roundF(&state, &block[0], &block[1], &block[2], &block[3], state.rounds);
            for (int r = 0; r < 4; r++) {
                last[r] = block[r];
            }
            k[3] = (block[0] & 0x000000FF);
            k[2] = (block[0] & 0x0000FF00) >> 8;
            k[1] = (block[0] & 0x00FF0000) >> 16;
            k[0] = (block[0] & 0xFF000000) >> 24;
            k[7] = (block[1] & 0x000000FF);
            k[6] = (block[1] & 0x0000FF00) >> 8;
            k[5] = (block[1] & 0x00FF0000) >> 16;
            k[4] = (block[1] & 0xFF000000) >> 24;
            k[11] = (block[2] & 0x000000FF);
            k[10] = (block[2] & 0x0000FF00) >> 8;
            k[9] = (block[2] & 0x00FF0000) >> 16;
            k[8] = (block[2] & 0xFF000000) >> 24;
            k[15] = (block[3] & 0x000000FF);
            k[14] = (block[3] & 0x0000FF00) >> 8;
            k[13] = (block[3] & 0x00FF0000) >> 16;
            k[12] = (block[3] & 0xFF000000) >> 24;
            fwrite(k, 1, v, outfile);
        }
    }
    else if(strcmp(mode, "d") == 0) {
        int blocks = (fsize - iv_length) / 16;
        int fsize_extra = (fsize - iv_length) % 16;
        int extra = 0;
        if (fsize_extra != 0) {
            blocks += 1;
        }
        fread(iv, 1, iv_length, infile);
        for (int i = 0; i < 4; i++) {
            last[i] = (iv[c] << 24) + (iv[c+1] << 16) + (iv[c+2] << 8) + iv[c+3];
            c += 4;
        }
        for (i = 0; i < (blocks); i++) {
            fread(data, 1, v, infile);
            block[0] = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
            block[1] = (data[4] << 24) + (data[5] << 16) + (data[6] << 8) + data[7];
            block[2] = (data[8] << 24) + (data[9] << 16) + (data[10] << 8) + data[11];
            block[3] = (data[12] << 24) + (data[13] << 16) + (data[14] << 8) + data[15];
            for (int r = 0; r < 4; r++) {
                next[r] = block[r];
            }
	    roundB(&state, &block[0], &block[1], &block[2], &block[3], state.rounds);
            for (int r = 0; r < 4; r++) {
                block[r] = block[r] ^ last[r];
                last[r] = next[r];
            }
            k[3] = (block[0] & 0x000000FF);
            k[2] = (block[0] & 0x0000FF00) >> 8;
            k[1] = (block[0] & 0x00FF0000) >> 16;
            k[0] = (block[0] & 0xFF000000) >> 24;
            k[7] = (block[1] & 0x000000FF);
            k[6] = (block[1] & 0x0000FF00) >> 8;
            k[5] = (block[1] & 0x00FF0000) >> 16;
            k[4] = (block[1] & 0xFF000000) >> 24;
            k[11] = (block[2] & 0x000000FF);
            k[10] = (block[2] & 0x0000FF00) >> 8;
            k[9] = (block[2] & 0x00FF0000) >> 16;
            k[8] = (block[2] & 0xFF000000) >> 24;
            k[15] = (block[3] & 0x000000FF);
            k[14] = (block[3] & 0x0000FF00) >> 8;
            k[13] = (block[3] & 0x00FF0000) >> 16;
            k[12] = (block[3] & 0xFF000000) >> 24;
            if (i == (blocks-1)) {
                int count = 0;
                int padcheck = k[15];
                int g = 15;
                for (int m = 0; m < padcheck; m++) {
                    if ((int)k[g] == padcheck) {
                        count += 1;
                    }
                    g = (g - 1);
                }
                if (count == padcheck) {
                    v = (v - count);
                }
            }
            fwrite(k, 1, v, outfile);
        }
    }
    fclose(outfile);
    fclose(outfile);
    fclose(infile);
}
