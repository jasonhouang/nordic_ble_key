/*
 * SM3 Hash alogrith 
 * thanks to Xyssl
 * author:goldboar
 * email:goldboar@163.com
 * 2011-10-26
 */

//Testing data from SM3 Standards
//http://www.oscca.gov.cn/News/201012/News_1199.htm 
// Sample 1
// Input:"abc"  
// Output:66c7f0f4 62eeedd9 d1f2d46b dc10e4e2 4167c487 5cf2f7a2 297da02b 8f4ba8e0

// Sample 2 
// Input:"abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd"
// Outpuf:debe9ff9 2275b8a1 38604889 c18e5a4d 6fdb70e5 387e5765 293dcba3 9c0c5732

#include "sm3.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
{                                                       \
    (n) = ( (unsigned long) (b)[(i)    ] << 24 )        \
    | ( (unsigned long) (b)[(i) + 1] << 16 )        \
    | ( (unsigned long) (b)[(i) + 2] <<  8 )        \
    | ( (unsigned long) (b)[(i) + 3]       );       \
}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}
#endif

int Atoi(char *str)
{
    if (*str == '\0')
        return 0;

    int res = 0;		// Initialize result
    int sign = 1;		// Initialize sign as positive
    int i = 0;			// Initialize index of first digit

    if (str[0] == '-')
    {
        sign = -1;
        i++;  			// Also update index of first digit
    }

    for (; str[i] != '\0'; ++i)
    {
        if ( str[i] <= '0' || str[i] >= '9')	// If string contain character it will terminate
            return 0; 

        res = res*10 + str[i] - '0';
    }

    return sign*res;
}
//#ifndef _DEBUG
//#define _DEBUG
//#endif
/*
 * SM3 context setup
 */
void sm3_starts( sm3_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x7380166F;
    ctx->state[1] = 0x4914B2B9;
    ctx->state[2] = 0x172442D7;
    ctx->state[3] = 0xDA8A0600;
    ctx->state[4] = 0xA96F30BC;
    ctx->state[5] = 0x163138AA;
    ctx->state[6] = 0xE38DEE4D;
    ctx->state[7] = 0xB0FB0E4E;

}

static void sm3_process( sm3_context *ctx, unsigned char data[64] )
{
    unsigned long SS1, SS2, TT1, TT2, W[68],W1[64];
    unsigned long A, B, C, D, E, F, G, H;
    unsigned long T[64];
    unsigned long Temp1,Temp2,Temp3,Temp4,Temp5;
    int j;
#ifdef _DEBUG
    int i;
#endif

    // 	for(j=0; j < 68; j++)
    // 		W[j] = 0;
    // 	for(j=0; j < 64; j++)
    // 		W1[j] = 0;

    for(j = 0; j < 16; j++)
        T[j] = 0x79CC4519;
    for(j =16; j < 64; j++)
        T[j] = 0x7A879D8A;

    GET_ULONG_BE( W[ 0], data,  0 );
    GET_ULONG_BE( W[ 1], data,  4 );
    GET_ULONG_BE( W[ 2], data,  8 );
    GET_ULONG_BE( W[ 3], data, 12 );
    GET_ULONG_BE( W[ 4], data, 16 );
    GET_ULONG_BE( W[ 5], data, 20 );
    GET_ULONG_BE( W[ 6], data, 24 );
    GET_ULONG_BE( W[ 7], data, 28 );
    GET_ULONG_BE( W[ 8], data, 32 );
    GET_ULONG_BE( W[ 9], data, 36 );
    GET_ULONG_BE( W[10], data, 40 );
    GET_ULONG_BE( W[11], data, 44 );
    GET_ULONG_BE( W[12], data, 48 );
    GET_ULONG_BE( W[13], data, 52 );
    GET_ULONG_BE( W[14], data, 56 );
    GET_ULONG_BE( W[15], data, 60 );

#ifdef _DEBUG 
    printf("Message with padding:\n");
    for(i=0; i< 8; i++)
        printf("%08x ",W[i]);
    printf("\n");
    for(i=8; i< 16; i++)
        printf("%08x ",W[i]);
    printf("\n");
#endif

#define FF0(x,y,z) (( (x) ^ (y) ^ (z)) & 0xFFFFFFFF)
#define FF1(x,y,z) ((((x) & (y)) | ( (x) & (z)) | ( (y) & (z))) & 0xFFFFFFFF)

#define GG0(x,y,z) (( (x) ^ (y) ^ (z)) & 0xFFFFFFFF)
#define GG1(x,y,z) ((((x) & (y)) | ( (~(x)) & (z)) ) & 0xFFFFFFFF )


#define  SHL(x,n) ((((x) ) << n) & 0xFFFFFFFF)
#define ROTL(x,n) ((SHL((x),n) | ((x) >> (32 - n))) & 0xFFFFFFFF)

#define P0(x) (((x) ^  ROTL((x),9) ^ ROTL((x),17))  & 0xFFFFFFFF)
#define P1(x) (((x) ^  ROTL((x),15) ^ ROTL((x),23)) & 0xFFFFFFFF)

    for(j = 16; j < 68; j++ )
    {
        //W[j] = P1( W[j-16] ^ W[j-9] ^ ROTL(W[j-3],15)) ^ ROTL(W[j - 13],7 ) ^ W[j-6];
        //Why thd release's result is different with the debug's ?
        //Below is okay. Interesting, Perhaps VC6 has a bug of Optimizaiton.
        Temp1 = W[j-16] ^ W[j-9];
        Temp2 = ROTL(W[j-3],15);
        Temp3 = Temp1 ^ Temp2;
        Temp4 = P1(Temp3);
        Temp5 =  ROTL(W[j - 13],7 ) ^ W[j-6];
        W[j] = Temp4 ^ Temp5;
    }

#ifdef _DEBUG 
    printf("Expanding message W0-67:\n");
    for(i=0; i<68; i++)
    {
        printf("%08lu ",W[i]);
        if(((i+1) % 8) == 0) printf("\n");
    }
    printf("\n");
#endif

    for(j =  0; j < 64; j++)
    {
        W1[j] = W[j] ^ W[j+4];
    }

#ifdef _DEBUG 
    printf("Expanding message W'0-63:\n");
    for(i=0; i<64; i++)
    {
        printf("%08lu ",W1[i]);
        if(((i+1) % 8) == 0) printf("\n");
    }
    printf("\n");
#endif

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];
    F = ctx->state[5];
    G = ctx->state[6];
    H = ctx->state[7];
#ifdef _DEBUG
    printf("j     A       B        C         D         E        F        G       H\n");
    printf("   %08lu %08lu %08lu %08lu %08lu %08lu %08lu %08lu\n",A,B,C,D,E,F,G,H);
#endif

    for(j =0; j < 16; j++)
    {
        SS1 = (ROTL((ROTL(A,12) + E + ROTL(T[j],j)) & 0xFFFFFFFF , 7)) & 0xFFFFFFFF;
        SS2 = (SS1 ^ ROTL(A,12)) & 0xFFFFFFFF;
        TT1 = (FF0(A,B,C) + D + SS2 + W1[j]) & 0xFFFFFFFF;
        TT2 = (GG0(E,F,G) + H + SS1 + W[j]) & 0xFFFFFFFF;
        D = C;
        C = ROTL(B,9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F,19);
        F = E;
        E = P0(TT2);
#ifdef _DEBUG 
        printf("%02d %08lu %08lu %08lu %08lu %08lu %08lu %08lu %08lu\n",j,A,B,C,D,E,F,G,H);
#endif
    }

    for(j =16; j < 32; j++)     //Wenchao: original "j<64"
    {
        SS1 = (ROTL((ROTL(A,12) + E + ROTL(T[j],j)) & 0xFFFFFFFF, 7)) & 0xFFFFFFFF;
        SS2 = (SS1 ^ ROTL(A,12)) & 0xFFFFFFFF;
        TT1 = (FF1(A,B,C) + D + SS2 + W1[j]) & 0xFFFFFFFF;
        TT2 = (GG1(E,F,G) + H + SS1 + W[j]) & 0xFFFFFFFF;
        D = C;
        C = ROTL(B,9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F,19);
        F = E;
        E = P0(TT2);
#ifdef _DEBUG 
        printf("%02d %08lu %08lu %08lu %08lu %08lu %08lu %08lu %08lu\n",j,A,B,C,D,E,F,G,H);
#endif	
    }

    ctx->state[0] ^= A;
    ctx->state[1] ^= B;
    ctx->state[2] ^= C;
    ctx->state[3] ^= D;
    ctx->state[4] ^= E;
    ctx->state[5] ^= F;
    ctx->state[6] ^= G;
    ctx->state[7] ^= H;
#ifdef _DEBUG 
    printf("   %08lu %08lu %08lu %08lu %08lu %08lu %08lu %08lu\n",ctx->state[0],ctx->state[1],ctx->state[2],
            ctx->state[3],ctx->state[4],ctx->state[5],ctx->state[6],ctx->state[7]);
#endif
}

/*
 * SM3 process buffer
 */
void sm3_update( sm3_context *ctx, unsigned char *input, int ilen )
{
    int fill;
    unsigned long left;

    if( ilen <= 0 )
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < (unsigned long) ilen )
        ctx->total[1]++;

    if( left && ilen >= fill )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );
        sm3_process( ctx, ctx->buffer );
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 )
    {
        sm3_process( ctx, input );
        input += 64;
        ilen  -= 64;
    }

    if( ilen > 0 )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, ilen );
    }
}

static const unsigned char sm3_padding[64] =
{
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SM3 final digest
 */
void sm3_finish( sm3_context *ctx, unsigned char output[4] )
{
    unsigned long last, padn;
    unsigned long high, low;
    unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
        | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_BE( high, msglen, 0 );
    PUT_ULONG_BE( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sm3_update( ctx, (unsigned char *) sm3_padding, padn );
    sm3_update( ctx, msglen, 8 );

    rstotp(ctx, output);
}

/*
 * output = SM3( input buffer )
 */
void sm3( unsigned char *input, int ilen,
        unsigned char output[4] )
{
    sm3_context ctx;

    sm3_starts( &ctx );
    sm3_update( &ctx, input, ilen );
    sm3_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sm3_context ) );
}
#if 0
/*
 * output = SM3( file contents )
 */
int sm3_file( char *path, unsigned char output[32] )
{
    FILE *f;
    size_t n;
    sm3_context ctx;
    unsigned char buf[1024];

    if( ( f = fopen( path, "rb" ) ) == NULL )
        return( 1 );

    sm3_starts( &ctx );

    while( ( n = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
        sm3_update( &ctx, buf, (int) n );

    sm3_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sm3_context ) );

    if( ferror( f ) != 0 )
    {
        fclose( f );
        return( 2 );
    }

    fclose( f );
    return( 0 );
}
#endif
/*
 * SM3 HMAC context setup
 */
void sm3_hmac_starts( sm3_context *ctx, unsigned char *key, int keylen )
{
    int i;
    unsigned char sum[32];

    if( keylen > 64 )
    {
        sm3( key, keylen, sum );
        keylen = 32;
        //keylen = ( is224 ) ? 28 : 32;
        key = sum;
    }

    memset( ctx->ipad, 0x36, 64 );
    memset( ctx->opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        ctx->ipad[i] = (unsigned char)( ctx->ipad[i] ^ key[i] );
        ctx->opad[i] = (unsigned char)( ctx->opad[i] ^ key[i] );
    }

    sm3_starts( ctx);
    sm3_update( ctx, ctx->ipad, 64 );

    memset( sum, 0, sizeof( sum ) );
}

/*
 * SM3 HMAC process buffer
 */
void sm3_hmac_update( sm3_context *ctx, unsigned char *input, int ilen )
{
    sm3_update( ctx, input, ilen );
}

/*
 * SM3 HMAC final digest
 */
void sm3_hmac_finish( sm3_context *ctx, unsigned char output[32] )
{
    int hlen;
    unsigned char tmpbuf[32];

    //is224 = ctx->is224;
    hlen =  32;

    sm3_finish( ctx, tmpbuf );
    sm3_starts( ctx );
    sm3_update( ctx, ctx->opad, 64 );
    sm3_update( ctx, tmpbuf, hlen );
    sm3_finish( ctx, output );

    memset( tmpbuf, 0, sizeof( tmpbuf ) );
}

/*
 * output = HMAC-SM#( hmac key, input buffer )
 */
void sm3_hmac( unsigned char *key, int keylen,
        unsigned char *input, int ilen,
        unsigned char output[32] )
{
    sm3_context ctx;

    sm3_hmac_starts( &ctx, key, keylen);
    sm3_hmac_update( &ctx, input, ilen );
    sm3_hmac_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sm3_context ) );
}

/**
 *  Output = genMessage (time4Intstr, seed32Hexstr)
*/
unsigned char* genMessage(char* time4Intstr, char* seed32Hexstr, unsigned char* msg)
{
    int time4Int = Atoi(time4Intstr);
    printf("time4Int: %d", time4Int);
    int list36[36] = {0};
    int list32[32] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2};
    int listtime4[4] = {time4Int / 1000000, (time4Int % 1000000) / 10000, (time4Int % 10000) / 100, time4Int % 100};
    //printf("listtime4:%d %d %d %d\r\n", listtime4[0], listtime4[1], listtime4[2], listtime4[3]);
    //unsigned char* msg = (unsigned char* )malloc(36);
    int i = 0;
    char hexByte[3];

    for (i = 0; i < strlen(seed32Hexstr); i = i + 2)
    {
        strncpy(hexByte, seed32Hexstr + i, 2);
        hexByte[2] = '\0';
        list32[i/2] = (int)strtol(hexByte, NULL, 16);
    }

    for(i = 0; i < 36; i++)
    {
        if (i < 4)
        {
            list36[i] = listtime4[i];
        } else {
            list36[i] = list32[i - 4];
        }
        msg[i] = (unsigned char) list36[i];
    }

    return msg;
}

unsigned char* genMessage1(uint32_t time4Int, const char* seed32Hexstr, unsigned char* msg)
{
    //printf("time4Int: %d", time4Int);
    int list36[36] = {0};
    int list32[32] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2};
    int listtime4[4] = {time4Int / 1000000, (time4Int % 1000000) / 10000, (time4Int % 10000) / 100, time4Int % 100};
    //printf("listtime4:%02x %02x %02x %02x\r\n", listtime4[0], listtime4[1], listtime4[2], listtime4[3]);
    //unsigned char* msg = (unsigned char* )malloc(36);
    //if (sizeof(msg) < 36) return NULL;
    int i = 0;
    char hexByte[3];

    for (i = 0; i < strlen(seed32Hexstr); i = i + 2)
    {
        strncpy(hexByte, seed32Hexstr + i, 2);
        hexByte[2] = '\0';
        list32[i/2] = (int)strtol(hexByte, NULL, 16);
    }

    for (i = 0; i < 36; i++)
    {
        if (i < 4)
        {
            list36[i] = listtime4[i];
        } else {
            list36[i] = list32[i - 4];
        }
        msg[i] = (unsigned char) list36[i];
    }

    return msg;
}
/**
 * Output = rstotp (sm3_o)
 */
void rstotp(sm3_context *ctx, unsigned char output[4])
{
    int i;
    unsigned long inti0, inti1, tmpInt;

    for (i = 0; i < 4; i++)
    {

        inti0 = (unsigned long) ctx->state[i<<1];
        inti1 = (unsigned long) ctx->state[(i<<1) + 1];

        tmpInt = (inti0 >> 24) + ((inti0 >> 16) & 0xFF) + ((inti0 >> 8) & 0xFF) + (inti0 & 0xFF);
        tmpInt = tmpInt + (inti1 >> 24) + ((inti1 >> 16) & 0xFF) + ((inti1 >> 8) & 0xFF) + (inti1 & 0xFF);
        output[i] = tmpInt & 0xFF;
    }

    return;
}

#if 0
void getMajorMinor(char* time4Intstr, char* seed32Hexstr, unsigned char majorMinor[8])
{
    char *input = "abc";
    int ilen = 36;
    unsigned char msgInput[36] = {0x0};

    input = genMessage(time4Intstr, seed32Hexstr, msgInput);
    sm3(input, ilen, majorMinor);

    return;
}

char* getMajorMinorArray(char* time4Intstr, char* seed32Hexstr)
{
    unsigned char *input;
    unsigned char majorMinor_malloc[8];
    unsigned char* majorMinor = majorMinor_malloc;//malloc(8 * sizeof(unsigned char));
    int ilen = 36;
    //    unsigned char majorMinor[8] = {0x0};
    input = genMessage(time4Intstr, seed32Hexstr);
    sm3(input, ilen, majorMinor);
    return (char *)majorMinor;//can not return local value
}
#endif

