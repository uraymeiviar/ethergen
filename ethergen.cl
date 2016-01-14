//author Uray Meiviar <uraymeiviar@tes.co.id>


#define KECCAK_ROUNDS 24
#define ROTL64(x, y)  (((x) << (y)) | ((x) >> (64 - (y))))
#define ROTL32xy(a,b) (uint2)( (a).x << (b) | (a).y >> (32-(b)) , (a).y << (b) | (a).x >> (32-(b)))
#define COPY(dst, src, count) for (uint i = 0; i != count; ++i) { (dst)[i] = (src)[i]; }

typedef union
{
    ulong u64[32 / sizeof(ulong)];
    uint  u32[32 / sizeof(uint)];
} bits256;

typedef union
{
    ulong u64[64 / sizeof(ulong)];
    uint  u32[64 / sizeof(uint)];
    uchar u8 [64];
} bits512;

__constant uint2 RC[24] = {
    (uint2)(0x00000001, 0x00000000),
    (uint2)(0x00008082, 0x00000000),
    (uint2)(0x0000808a, 0x80000000),
    (uint2)(0x80008000, 0x80000000),
    (uint2)(0x0000808b, 0x00000000),
    (uint2)(0x80000001, 0x00000000),
    (uint2)(0x80008081, 0x80000000),
    (uint2)(0x00008009, 0x80000000),
    (uint2)(0x0000008a, 0x00000000),
    (uint2)(0x00000088, 0x00000000),
    (uint2)(0x80008009, 0x00000000),
    (uint2)(0x8000000a, 0x00000000),
    (uint2)(0x8000808b, 0x00000000),
    (uint2)(0x0000008b, 0x80000000),
    (uint2)(0x00008089, 0x80000000),
    (uint2)(0x00008003, 0x80000000),
    (uint2)(0x00008002, 0x80000000),
    (uint2)(0x00000080, 0x80000000),
    (uint2)(0x0000800a, 0x00000000),
    (uint2)(0x8000000a, 0x80000000),
    (uint2)(0x80008081, 0x80000000),
    (uint2)(0x00008080, 0x80000000),
    (uint2)(0x80000001, 0x00000000),
    (uint2)(0x80008008, 0x80000000),
};

static void keccak_final256(uint2* st)
{
    uint2* ps = &st[25];

    ps[0] = st[0] ^ st[0 + 5] ^ st[0 + 10] ^ st[0 + 15] ^ st[0 + 20];
    ps[1] = st[1] ^ st[1 + 5] ^ st[1 + 10] ^ st[1 + 15] ^ st[1 + 20];
    ps[2] = st[2] ^ st[2 + 5] ^ st[2 + 10] ^ st[2 + 15] ^ st[2 + 20];
    ps[3] = st[3] ^ st[3 + 5] ^ st[3 + 10] ^ st[3 + 15] ^ st[3 + 20];
    ps[4] = st[4] ^ st[4 + 5] ^ st[4 + 10] ^ st[4 + 15] ^ st[4 + 20];
    
    ps[5] = ps[4] ^ ROTL32xy(ps[1], 1);
    ps[6] = ps[0] ^ ROTL32xy(ps[2], 1);
    ps[7] = ps[1] ^ ROTL32xy(ps[3], 1);
    ps[8] = ps[2] ^ ROTL32xy(ps[4], 1);
    ps[9] = ps[3] ^ ROTL32xy(ps[0], 1);
    
    st[ 0+0] ^= ps[5];
    st[ 5+1] ^= ps[6];
    st[10+2] ^= ps[7];
    st[15+3] ^= ps[8];
    st[20+4] ^= ps[9];
    
    // Rho Pi
    ps[0] = st[ 0];
    ps[3] = ROTL32xy(st[18], 21);
    ps[4] = ROTL32xy(st[24], 14);
    ps[2] = ROTL32xy(st[12], 43);
    ps[1] = ROTL32xy(st[ 6], 44);

    st[0] = bitselect(ps[0] ^ ps[2], ps[0], ps[1]);
    st[1] = bitselect(ps[1] ^ ps[3], ps[1], ps[2]);
    st[2] = bitselect(ps[2] ^ ps[4], ps[2], ps[3]);
    st[3] = bitselect(ps[3] ^ ps[0], ps[3], ps[4]);
    
    //  Iota
    st[0] ^= RC[23];
}

static void keccak_final512(uint2* st)
{
    uint2* ps = &st[25];
    
    ps[0] = st[0] ^ st[0 + 5] ^ st[0 + 10] ^ st[0 + 15] ^ st[0 + 20];
    ps[1] = st[1] ^ st[1 + 5] ^ st[1 + 10] ^ st[1 + 15] ^ st[1 + 20];
    ps[2] = st[2] ^ st[2 + 5] ^ st[2 + 10] ^ st[2 + 15] ^ st[2 + 20];
    ps[3] = st[3] ^ st[3 + 5] ^ st[3 + 10] ^ st[3 + 15] ^ st[3 + 20];
    ps[4] = st[4] ^ st[4 + 5] ^ st[4 + 10] ^ st[4 + 15] ^ st[4 + 20];
    
    ps[5] = ps[4] ^ ROTL32xy(ps[1], 1);
    ps[6] = ps[0] ^ ROTL32xy(ps[2], 1);
    ps[7] = ps[1] ^ ROTL32xy(ps[3], 1);
    ps[8] = ps[2] ^ ROTL32xy(ps[4], 1);
    ps[9] = ps[3] ^ ROTL32xy(ps[0], 1);
    
    st[ 0+0] ^= ps[5];
    st[ 0+3] ^= ps[8];
    st[ 5+1] ^= ps[6];
    st[ 5+4] ^= ps[9];
    st[10+0] ^= ps[5];
    st[10+2] ^= ps[7];
    st[15+1] ^= ps[6];
    st[15+3] ^= ps[8];
    st[20+2] ^= ps[7];
    st[20+4] ^= ps[9];
    
    // Rho Pi
    ps[ 0] = st[ 0];
    ps[ 7] = ROTL32xy(st[10],  3);
    ps[ 3] = ROTL32xy(st[18], 21);
    ps[ 5] = ROTL32xy(st[ 3], 28);
    ps[ 8] = ROTL32xy(st[16], 45);
    ps[ 4] = ROTL32xy(st[24], 14);
    ps[ 2] = ROTL32xy(st[12], 43);
    ps[ 9] = ROTL32xy(st[22], 61);
    ps[ 6] = ROTL32xy(st[ 9], 20);
    ps[ 1] = ROTL32xy(st[ 6], 44);

    st[0] = bitselect(ps[0] ^ ps[2], ps[0], ps[1]);
    st[1] = bitselect(ps[1] ^ ps[3], ps[1], ps[2]);
    st[2] = bitselect(ps[2] ^ ps[4], ps[2], ps[3]);
    st[3] = bitselect(ps[3] ^ ps[0], ps[3], ps[4]);
    st[4] = bitselect(ps[4] ^ ps[1], ps[4], ps[0]);
    st[5] = bitselect(ps[5] ^ ps[7], ps[5], ps[6]);
    st[6] = bitselect(ps[6] ^ ps[8], ps[6], ps[7]);
    st[7] = bitselect(ps[7] ^ ps[9], ps[7], ps[8]);
    st[8] = bitselect(ps[8] ^ ps[5], ps[8], ps[9]);
    
    //  Iota
    st[0] ^= RC[23];
}

static void keccak_round(uint2* st, int round)
{
    uint2* ps = &st[25];
    
    for (uint r = 0; r < round; r++)
    {
        ps[0] = st[0] ^ st[0 + 5] ^ st[0 + 10] ^ st[0 + 15] ^ st[0 + 20];
        ps[1] = st[1] ^ st[1 + 5] ^ st[1 + 10] ^ st[1 + 15] ^ st[1 + 20];
        ps[2] = st[2] ^ st[2 + 5] ^ st[2 + 10] ^ st[2 + 15] ^ st[2 + 20];
        ps[3] = st[3] ^ st[3 + 5] ^ st[3 + 10] ^ st[3 + 15] ^ st[3 + 20];
        ps[4] = st[4] ^ st[4 + 5] ^ st[4 + 10] ^ st[4 + 15] ^ st[4 + 20];
        
        ps[5] = ps[4] ^ ROTL32xy(ps[1], 1);
        ps[6] = ps[0] ^ ROTL32xy(ps[2], 1);
        ps[7] = ps[1] ^ ROTL32xy(ps[3], 1);
        ps[8] = ps[2] ^ ROTL32xy(ps[4], 1);
        ps[9] = ps[3] ^ ROTL32xy(ps[0], 1);
        
        st[ 0+0] ^= ps[5];
        st[ 0+1] ^= ps[6];
        st[ 0+2] ^= ps[7];
        st[ 0+3] ^= ps[8];
        st[ 0+4] ^= ps[9];
        
        st[ 5+0] ^= ps[5];
        st[ 5+1] ^= ps[6];
        st[ 5+2] ^= ps[7];
        st[ 5+3] ^= ps[8];
        st[ 5+4] ^= ps[9];
        
        st[10+0] ^= ps[5];
        st[10+1] ^= ps[6];
        st[10+2] ^= ps[7];
        st[10+3] ^= ps[8];
        st[10+4] ^= ps[9];
        
        st[15+0] ^= ps[5];
        st[15+1] ^= ps[6];
        st[15+2] ^= ps[7];
        st[15+3] ^= ps[8];
        st[15+4] ^= ps[9];
        
        st[20+0] ^= ps[5];
        st[20+1] ^= ps[6];
        st[20+2] ^= ps[7];
        st[20+3] ^= ps[8];
        st[20+4] ^= ps[9];
        
        // Rho Pi
        ps[ 0] = st[ 0];
        ps[10] = ROTL32xy(st[ 1],  1);
        ps[ 7] = ROTL32xy(st[10],  3);
        ps[11] = ROTL32xy(st[ 7],  6);
        ps[17] = ROTL32xy(st[11], 10);
        ps[18] = ROTL32xy(st[17], 15);
        ps[ 3] = ROTL32xy(st[18], 21);
        ps[ 5] = ROTL32xy(st[ 3], 28);
        ps[16] = ROTL32xy(st[ 5], 36);
        ps[ 8] = ROTL32xy(st[16], 45);
        ps[21] = ROTL32xy(st[ 8], 55);
        ps[24] = ROTL32xy(st[21],  2);
        ps[ 4] = ROTL32xy(st[24], 14);
        ps[15] = ROTL32xy(st[ 4], 27);
        ps[23] = ROTL32xy(st[15], 41);
        ps[19] = ROTL32xy(st[23], 56);
        ps[13] = ROTL32xy(st[19],  8);
        ps[12] = ROTL32xy(st[13], 25);
        ps[ 2] = ROTL32xy(st[12], 43);
        ps[20] = ROTL32xy(st[ 2], 62);
        ps[14] = ROTL32xy(st[20], 18);
        ps[22] = ROTL32xy(st[14], 39);
        ps[ 9] = ROTL32xy(st[22], 61);
        ps[ 6] = ROTL32xy(st[ 9], 20);
        ps[ 1] = ROTL32xy(st[ 6], 44);

        st[ 0] = bitselect(ps[ 0] ^ ps[ 2], ps[ 0], ps[ 1]);
        st[ 1] = bitselect(ps[ 1] ^ ps[ 3], ps[ 1], ps[ 2]);
        st[ 2] = bitselect(ps[ 2] ^ ps[ 4], ps[ 2], ps[ 3]);
        st[ 3] = bitselect(ps[ 3] ^ ps[ 0], ps[ 3], ps[ 4]);
        st[ 4] = bitselect(ps[ 4] ^ ps[ 1], ps[ 4], ps[ 0]);
        st[ 5] = bitselect(ps[ 5] ^ ps[ 7], ps[ 5], ps[ 6]);
        st[ 6] = bitselect(ps[ 6] ^ ps[ 8], ps[ 6], ps[ 7]);
        st[ 7] = bitselect(ps[ 7] ^ ps[ 9], ps[ 7], ps[ 8]);
        st[ 8] = bitselect(ps[ 8] ^ ps[ 5], ps[ 8], ps[ 9]);
        st[ 9] = bitselect(ps[ 9] ^ ps[ 6], ps[ 9], ps[ 5]);
        st[10] = bitselect(ps[10] ^ ps[12], ps[10], ps[11]);
        st[11] = bitselect(ps[11] ^ ps[13], ps[11], ps[12]);
        st[12] = bitselect(ps[12] ^ ps[14], ps[12], ps[13]);
        st[13] = bitselect(ps[13] ^ ps[10], ps[13], ps[14]);
        st[14] = bitselect(ps[14] ^ ps[11], ps[14], ps[10]);
        st[15] = bitselect(ps[15] ^ ps[17], ps[15], ps[16]);
        st[16] = bitselect(ps[16] ^ ps[18], ps[16], ps[17]);
        st[17] = bitselect(ps[17] ^ ps[19], ps[17], ps[18]);
        st[18] = bitselect(ps[18] ^ ps[15], ps[18], ps[19]);
        st[19] = bitselect(ps[19] ^ ps[16], ps[19], ps[15]);
        st[20] = bitselect(ps[20] ^ ps[22], ps[20], ps[21]);
        st[21] = bitselect(ps[21] ^ ps[23], ps[21], ps[22]);
        st[22] = bitselect(ps[22] ^ ps[24], ps[22], ps[23]);
        st[23] = bitselect(ps[23] ^ ps[20], ps[23], ps[24]);
        st[24] = bitselect(ps[24] ^ ps[21], ps[24], ps[20]);
        
        //  Iota
        st[0] ^= RC[r];
    }
}

static void SHA3_96B_256(uchar* ret, uchar const* data)
{
    uint2 st[50];
    for (uint i = 0; i < 25; i++)
    {
        st[i].x = 0;
        st[i].y = 0;
    }
    uchar* a = (uchar*)st;
    const ulong* in64 = (const ulong*)data;
    
    a[ 96] ^= 0x01;
    a[135] ^= 0x80;
    for(uint i=0 ; i<96/8 ; i++)
    {
        st[i] ^= in64[i];
    }
    keccak_round(st,KECCAK_ROUNDS-1);
    keccak_final256(st);
    
    COPY(ret,a,32);
}

static void SHA3_40B_512(uchar* ret, uchar const* data)
{
    uint2 st[50];
    for (uint i = 0; i < 25; i++)
    {
        st[i].x = 0;
        st[i].y = 0;
    }
    uchar* a = (uchar*)st;
    const ulong* in64 = (const ulong*)data;
    
    a[40] ^= 0x01;
    a[71] ^= 0x80;
    for(size_t i=0 ; i<40/8 ; i++)
    {
        st[i] ^= in64[i];
    }
    keccak_round(st,KECCAK_ROUNDS-1);
    keccak_final512(st);
    
    COPY(ret,a,64);
}

//                64|         64|         64|         64| = 256 bytes x 1,048,576 = 256MB
//     +------------+-----------+-----------+-----------+
//SMIX |           0|          1|          2|          3|
//pre  |header|nonce|           |           |     |nonce|
//post |            |mix  |     |           |hash |nonce|

__kernel void prehash(
        __global bits512* hashpad,
        __constant bits256 const* header,
        ulong startNonce )
{

}

__kernel void finalhash(
        __global bits512* hashpad,
        __constant bits256 const* target)
{

}


