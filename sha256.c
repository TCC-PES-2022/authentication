#include "sha256.h"
#include <stdio.h>

const uint32_t hashStart[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
			       0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

const uint32_t k[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
			0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
			0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
			0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
			0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
			0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
			0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
			0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
			0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
			0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
			0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
			0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
			0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
			0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
			0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
			0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

static uint32_t
ep0(uint32_t x)
{
	uint32_t rotate_right2, rotate_right13, rotate_right22;

	rotate_right2  = (x >> 2)  | (x << 30);
	rotate_right13 = (x >> 13) | (x << 19);
	rotate_right22 = (x >> 22) | (x << 10);

	return rotate_right2 ^ rotate_right13 ^ rotate_right22;
}

static uint32_t
ep1(uint32_t x)
{
	uint32_t rotate_right6, rotate_right11, rotate_right25;

	rotate_right6  = (x >> 6)  | (x << 26);
	rotate_right11 = (x >> 11) | (x << 21);
	rotate_right25 = (x >> 25) | (x << 7);

	return rotate_right6 ^ rotate_right11 ^ rotate_right25;
}

static uint32_t
sigma0(uint32_t x)
{
	uint32_t rotate_right7, rotate_right18, shift_right3;

	rotate_right7  = (x >> 7)  | (x << 25);
	rotate_right18 = (x >> 18) | (x << 14);
	shift_right3  = (x >> 3);

	return rotate_right7 ^ rotate_right18 ^ shift_right3;
}

static uint32_t
sigma1(uint32_t x)
{
	uint32_t rotate_right17, rotate_right19, shift_right10;

	rotate_right17 = (x >> 17) | (x << 15);
	rotate_right19 = (x >> 19) | (x << 13);
	shift_right10 = (x >> 10);

	return rotate_right17 ^ rotate_right19 ^ shift_right10;
}

static uint32_t
major(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) ^ (x & z) ^ (y & z);
}

static uint32_t
ch(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) ^ (~x & z);
}

static void
sha256_transform(sha256_ctx *ctx, const uint8_t data[])
{
	uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

	for (i = 0, j = 0; i < 16; i++, j += 4) {
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | 
		(data[j + 2] << 8) | (data[j + 3]);
	}
	for (; i < 64; i++) {
		m[i] = sigma1(m[i - 2]) + m[i - 7] + sigma0(m[i - 15]) + 
		m[i - 16];
	}

	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];

	for (i = 0; i < 64; i++) {
		t1 = h + ep1(e) + ch(e, f, g) + k[i] + m[i];
		t2 = ep0(a) + major(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;

	return;
}

static void
sha256_init(sha256_ctx *ctx)
{
	int i;

	ctx->datalen = 0;
	ctx->bitlen = 0;
	for (i = 0; i < 8; i++)
		ctx->state[i] = hashStart[i];

	return;
}

static void
sha256_update(sha256_ctx *ctx, const uint8_t data[], size_t len)
{
	uint8_t i;

	for (i = 0; i < len; i++) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			sha256_transform(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}

	return;
}

static void
sha256_final(sha256_ctx *ctx, uint8_t hash[])
{
	uint8_t i;

	i = ctx->datalen;

	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56)
			ctx->data[i++] = 0x00;
	}
	else {
		ctx->data[i++] = 0x80;
		while (i < 64)
			ctx->data[i++] = 0x00;
		sha256_transform(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}

	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	sha256_transform(ctx, ctx->data);

	for (i = 0; i < 4; i++) {
		hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
	}
	
	return;
}

static void
sha256_processing(const uint8_t data[], size_t len, uint8_t hash[])
{
	sha256_ctx ctx;

	sha256_init(&ctx);
	sha256_update(&ctx, data, len);
	sha256_final(&ctx, hash);

	return;
}

static void
convert(char *result_hash, uint8_t hash[])
{
	int i;
	char shash[4];

	sprintf(shash, "%x", hash[0]);
	strncpy(result_hash, shash, 2);
	for (i = 1; i < SHA256_BLOCK_SIZE; i++) {
		sprintf(shash, "%x", hash[i]);
		strncat(result_hash, shash, 2);
	}

	return;
}

void
sha256(const uint8_t *input, char *output)
{
	uint8_t hash[SHA256_BLOCK_SIZE*2];

	sha256_processing(input, sizeof(input), hash); 
	convert(output, hash);

	return;
}
