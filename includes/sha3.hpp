#ifndef SHA3_H
#define SHA3_H

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

#ifndef KECCAKF_ROUNDS
#define KECCAKF_ROUNDS 24
#endif

#ifndef ROTL64
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))
#endif

static const uint64_t RC[24] = {
    0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
    0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
    0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
    0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
    0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
    0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
    0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
    0x8000000000008080, 0x0000000080000001, 0x8000000080008008
};
/*
static void keccakf_sse(uint64_t st[32])
{
    alignas(16) uint64_t t[8];
    alignas(16) uint64_t bc[16];
    alignas(16) uint64_t pst[32];
    
    __m128i* st128 = (__m128i*)st;
    __m128i* ps128 = (__m128i*)pst;
    __m128i* bc128 = (__m128i*)bc;
    
    for (int r = 0; r < KECCAKF_ROUNDS; r++)
    {
        bc[0] = st[0] ^ st[0 + 5] ^ st[0 + 10] ^ st[0 + 15] ^ st[0 + 20];
        bc[1] = st[1] ^ st[1 + 5] ^ st[1 + 10] ^ st[1 + 15] ^ st[1 + 20];
        bc[2] = st[2] ^ st[2 + 5] ^ st[2 + 10] ^ st[2 + 15] ^ st[2 + 20];
        bc[3] = st[3] ^ st[3 + 5] ^ st[3 + 10] ^ st[3 + 15] ^ st[3 + 20];
        bc[4] = st[4] ^ st[4 + 5] ^ st[4 + 10] ^ st[4 + 15] ^ st[4 + 20];
        
        t[0] = bc[4] ^ ROTL64(bc[1], 1);
        t[1] = bc[0] ^ ROTL64(bc[2], 1);
        t[2] = bc[1] ^ ROTL64(bc[3], 1);
        t[3] = bc[2] ^ ROTL64(bc[4], 1);
        t[4] = bc[3] ^ ROTL64(bc[0], 1);
        
        bc128[0] = _mm_set_epi64x(t[1],t[0]);
        bc128[1] = _mm_set_epi64x(t[3],t[2]);
        bc128[2] = _mm_set_epi64x(t[0],t[4]);
        bc128[3] = _mm_set_epi64x(t[2],t[1]);
        bc128[4] = _mm_set_epi64x(t[4],t[3]);
        
        st128[0] = _mm_xor_si128(st128[0], bc128[0]);
        st128[1] = _mm_xor_si128(st128[1], bc128[1]);
        st128[2] = _mm_xor_si128(st128[2], bc128[2]);
        st128[3] = _mm_xor_si128(st128[3], bc128[3]);
        st128[4] = _mm_xor_si128(st128[4], bc128[4]);
        
        st128[5] = _mm_xor_si128(st128[5], bc128[0]);
        st128[6] = _mm_xor_si128(st128[6], bc128[1]);
        st128[7] = _mm_xor_si128(st128[7], bc128[2]);
        st128[8] = _mm_xor_si128(st128[8], bc128[3]);
        st128[9] = _mm_xor_si128(st128[9], bc128[4]);
        
        st128[10] = _mm_xor_si128(st128[10], bc128[0]);
        st128[11] = _mm_xor_si128(st128[11], bc128[1]);
        
        st[20+4] ^= t[4];
        
        // Rho Pi
        pst[ 0] = st[ 0];
        pst[10] = ROTL64(st[ 1],  1);
        pst[ 7] = ROTL64(st[10],  3);
        pst[11] = ROTL64(st[ 7],  6);
        pst[17] = ROTL64(st[11], 10);
        pst[18] = ROTL64(st[17], 15);
        pst[ 3] = ROTL64(st[18], 21);
        pst[ 5] = ROTL64(st[ 3], 28);
        pst[16] = ROTL64(st[ 5], 36);
        pst[ 8] = ROTL64(st[16], 45);
        pst[21] = ROTL64(st[ 8], 55);
        pst[24] = ROTL64(st[21],  2);
        pst[ 4] = ROTL64(st[24], 14);
        pst[15] = ROTL64(st[ 4], 27);
        pst[23] = ROTL64(st[15], 41);
        pst[19] = ROTL64(st[23], 56);
        pst[13] = ROTL64(st[19],  8);
        pst[12] = ROTL64(st[13], 25);
        pst[ 2] = ROTL64(st[12], 43);
        pst[20] = ROTL64(st[ 2], 62);
        pst[14] = ROTL64(st[20], 18);
        pst[22] = ROTL64(st[14], 39);
        pst[ 9] = ROTL64(st[22], 61);
        pst[ 6] = ROTL64(st[ 9], 20);
        pst[ 1] = ROTL64(st[ 6], 44);
        
        st[ 0] = (~pst[ 0 + 1]) & pst[ 0 + 2];
        st[ 1] = (~pst[ 0 + 2]) & pst[ 0 + 3];
        st[ 2] = (~pst[ 0 + 3]) & pst[ 0 + 4];
        st[ 3] = (~pst[ 0 + 4]) & pst[ 0 + 0];
        st[ 4] = (~pst[ 0 + 0]) & pst[ 0 + 1];
        
        st[ 5] = (~pst[ 5 + 1]) & pst[ 5 + 2];
        st[ 6] = (~pst[ 5 + 2]) & pst[ 5 + 3];
        st[ 7] = (~pst[ 5 + 3]) & pst[ 5 + 4];
        st[ 8] = (~pst[ 5 + 4]) & pst[ 5 + 0];
        st[ 9] = (~pst[ 5 + 0]) & pst[ 5 + 1];
        
        st[10] = (~pst[10 + 1]) & pst[10 + 2];
        st[11] = (~pst[10 + 2]) & pst[10 + 3];
        st[12] = (~pst[10 + 3]) & pst[10 + 4];
        st[13] = (~pst[10 + 4]) & pst[10 + 0];
        st[14] = (~pst[10 + 0]) & pst[10 + 1];
        
        st[15] = (~pst[15 + 1]) & pst[15 + 2];
        st[16] = (~pst[15 + 2]) & pst[15 + 3];
        st[17] = (~pst[15 + 3]) & pst[15 + 4];
        st[18] = (~pst[15 + 4]) & pst[15 + 0];
        st[19] = (~pst[15 + 0]) & pst[15 + 1];
        
        st[20] = (~pst[20 + 1]) & pst[20 + 2];
        st[21] = (~pst[20 + 2]) & pst[20 + 3];
        st[22] = (~pst[20 + 3]) & pst[20 + 4];
        st[23] = (~pst[20 + 4]) & pst[20 + 0];
        st[24] = (~pst[20 + 0]) & pst[20 + 1];
        
        st128[0] = _mm_xor_si128(st128[0], ps128[0]);
        st128[1] = _mm_xor_si128(st128[1], ps128[1]);
        st128[2] = _mm_xor_si128(st128[2], ps128[2]);
        st128[3] = _mm_xor_si128(st128[3], ps128[3]);
        st128[4] = _mm_xor_si128(st128[4], ps128[4]);
        
        st128[5] = _mm_xor_si128(st128[5], ps128[5]);
        st128[6] = _mm_xor_si128(st128[6], ps128[6]);
        st128[7] = _mm_xor_si128(st128[7], ps128[7]);
        st128[8] = _mm_xor_si128(st128[8], ps128[8]);
        st128[9] = _mm_xor_si128(st128[9], ps128[9]);
        
        st128[10] = _mm_xor_si128(st128[10], ps128[10]);
        st128[11] = _mm_xor_si128(st128[11], ps128[11]);
        st[24] = pst[24] ^ (~pst[20 + 0]) & pst[20 + 1];
        
        //  Iota
        st[0] ^= RC[r];
    }
}
*/

static void keccakf(uint64_t* st)
{
    uint64_t* pst = &st[25];
    
    for (int r = 0; r < KECCAKF_ROUNDS; r++)
    {
        pst[0] = st[0] ^ st[0 + 5] ^ st[0 + 10] ^ st[0 + 15] ^ st[0 + 20];
        pst[1] = st[1] ^ st[1 + 5] ^ st[1 + 10] ^ st[1 + 15] ^ st[1 + 20];
        pst[2] = st[2] ^ st[2 + 5] ^ st[2 + 10] ^ st[2 + 15] ^ st[2 + 20];
        pst[3] = st[3] ^ st[3 + 5] ^ st[3 + 10] ^ st[3 + 15] ^ st[3 + 20];
        pst[4] = st[4] ^ st[4 + 5] ^ st[4 + 10] ^ st[4 + 15] ^ st[4 + 20];
        
        pst[5] = pst[4] ^ ROTL64(pst[1], 1);
        pst[6] = pst[0] ^ ROTL64(pst[2], 1);
        pst[7] = pst[1] ^ ROTL64(pst[3], 1);
        pst[8] = pst[2] ^ ROTL64(pst[4], 1);
        pst[9] = pst[3] ^ ROTL64(pst[0], 1);
        
        st[ 0+0] ^= pst[5];
        st[ 0+1] ^= pst[6];
        st[ 0+2] ^= pst[7];
        st[ 0+3] ^= pst[8];
        st[ 0+4] ^= pst[9];
        
        st[ 5+0] ^= pst[5];
        st[ 5+1] ^= pst[6];
        st[ 5+2] ^= pst[7];
        st[ 5+3] ^= pst[8];
        st[ 5+4] ^= pst[9];
        
        st[10+0] ^= pst[5];
        st[10+1] ^= pst[6];
        st[10+2] ^= pst[7];
        st[10+3] ^= pst[8];
        st[10+4] ^= pst[9];
        
        st[15+0] ^= pst[5];
        st[15+1] ^= pst[6];
        st[15+2] ^= pst[7];
        st[15+3] ^= pst[8];
        st[15+4] ^= pst[9];
        
        st[20+0] ^= pst[5];
        st[20+1] ^= pst[6];
        st[20+2] ^= pst[7];
        st[20+3] ^= pst[8];
        st[20+4] ^= pst[9];
        
        // Rho Pi
        pst[ 0] = st[ 0];
        pst[10] = ROTL64(st[ 1],  1);
        pst[ 7] = ROTL64(st[10],  3);
        pst[11] = ROTL64(st[ 7],  6);
        pst[17] = ROTL64(st[11], 10);
        pst[18] = ROTL64(st[17], 15);
        pst[ 3] = ROTL64(st[18], 21);
        pst[ 5] = ROTL64(st[ 3], 28);
        pst[16] = ROTL64(st[ 5], 36);
        pst[ 8] = ROTL64(st[16], 45);
        pst[21] = ROTL64(st[ 8], 55);
        pst[24] = ROTL64(st[21],  2);
        pst[ 4] = ROTL64(st[24], 14);
        pst[15] = ROTL64(st[ 4], 27);
        pst[23] = ROTL64(st[15], 41);
        pst[19] = ROTL64(st[23], 56);
        pst[13] = ROTL64(st[19],  8);
        pst[12] = ROTL64(st[13], 25);
        pst[ 2] = ROTL64(st[12], 43);
        pst[20] = ROTL64(st[ 2], 62);
        pst[14] = ROTL64(st[20], 18);
        pst[22] = ROTL64(st[14], 39);
        pst[ 9] = ROTL64(st[22], 61);
        pst[ 6] = ROTL64(st[ 9], 20);
        pst[ 1] = ROTL64(st[ 6], 44);
        
        st[ 0] = pst[ 0] ^ ((~pst[ 0 + 1]) & pst[ 0 + 2]);
        st[ 1] = pst[ 1] ^ ((~pst[ 0 + 2]) & pst[ 0 + 3]);
        st[ 2] = pst[ 2] ^ ((~pst[ 0 + 3]) & pst[ 0 + 4]);
        st[ 3] = pst[ 3] ^ ((~pst[ 0 + 4]) & pst[ 0 + 0]);
        st[ 4] = pst[ 4] ^ ((~pst[ 0 + 0]) & pst[ 0 + 1]);
        
        st[ 5] = pst[ 5] ^ ((~pst[ 5 + 1]) & pst[ 5 + 2]);
        st[ 6] = pst[ 6] ^ ((~pst[ 5 + 2]) & pst[ 5 + 3]);
        st[ 7] = pst[ 7] ^ ((~pst[ 5 + 3]) & pst[ 5 + 4]);
        st[ 8] = pst[ 8] ^ ((~pst[ 5 + 4]) & pst[ 5 + 0]);
        st[ 9] = pst[ 9] ^ ((~pst[ 5 + 0]) & pst[ 5 + 1]);
        
        st[10] = pst[10] ^ ((~pst[10 + 1]) & pst[10 + 2]);
        st[11] = pst[11] ^ ((~pst[10 + 2]) & pst[10 + 3]);
        st[12] = pst[12] ^ ((~pst[10 + 3]) & pst[10 + 4]);
        st[13] = pst[13] ^ ((~pst[10 + 4]) & pst[10 + 0]);
        st[14] = pst[14] ^ ((~pst[10 + 0]) & pst[10 + 1]);
        
        st[15] = pst[15] ^ ((~pst[15 + 1]) & pst[15 + 2]);
        st[16] = pst[16] ^ ((~pst[15 + 2]) & pst[15 + 3]);
        st[17] = pst[17] ^ ((~pst[15 + 3]) & pst[15 + 4]);
        st[18] = pst[18] ^ ((~pst[15 + 4]) & pst[15 + 0]);
        st[19] = pst[19] ^ ((~pst[15 + 0]) & pst[15 + 1]);
        
        st[20] = pst[20] ^ ((~pst[20 + 1]) & pst[20 + 2]);
        st[21] = pst[21] ^ ((~pst[20 + 2]) & pst[20 + 3]);
        st[22] = pst[22] ^ ((~pst[20 + 3]) & pst[20 + 4]);
        st[23] = pst[23] ^ ((~pst[20 + 4]) & pst[20 + 0]);
        st[24] = pst[24] ^ ((~pst[20 + 0]) & pst[20 + 1]);
        
        //  Iota
        st[0] ^= RC[r];
    }
}

static inline void hash(uint8_t* out, size_t outlen,
                 const uint8_t* in, size_t inlen,
                 size_t rate)
{
    alignas(8) uint64_t st[50] = {0};
    uint8_t* a = (uint8_t*)st;
    const uint64_t* in64 = (const uint64_t*)in;
    
    while (inlen >= rate)
    {
        for(size_t i=0 ; i<rate/8 ; i++)
        {
            st[i] ^= in64[i];
        }
        keccakf(st);
        in += rate;
        inlen -= rate;
    }
    
    a[inlen] ^= 0x01;
    a[rate - 1] ^= 0x80;
    for(size_t i=0 ; i<inlen/8 ; i++)
    {
        st[i] ^= in64[i];
    }
    keccakf(st);
    
    while (outlen >= rate)
    {
        for(size_t i=0 ; i<rate ; i++)
        {
            a[i] = in[i];
        }
        keccakf(st);
        out += rate;
        outlen -= rate;
    }
    
    for(size_t i=0 ; i<outlen ; i++)
    {
        out[i] = a[i];
    }
}

static inline void SHA3_256(uint8_t* ret, uint8_t const* data, size_t const size)
{
    hash(ret, 32, data, size, 200 - (256 / 4)); //136
}

static inline void SHA3_512(uint8_t* ret, uint8_t const* data, size_t const size)
{
    hash(ret, 64, data, size, 200 - (512 / 4)); //72
}

#endif
