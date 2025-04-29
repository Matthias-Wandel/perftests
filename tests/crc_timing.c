//----------------------------------------------------------------------------
// Different CRC implementations that can mess up branch predictions.
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <string.h>
#include "perftest.h"

//----------------------------------------------------------------------------
// Function to initialize the CRC32 lookup table
//----------------------------------------------------------------------------
uint32_t crc32_table[256];
void init_crc32_table(void)
{
    uint32_t polynomial = 0xEDB88320;
    int i;
    for (i = 0; i < 256; i++) {
        uint32_t crc = i;
        int j;
        for (j = 0; j < 8; j++)
            crc = (crc & 1) ? (polynomial ^ (crc >> 1)) : (crc >> 1);
        crc32_table[i] = crc;
    }
}

//----------------------------------------------------------------------------
// Function to compute CRC32 on a buffer
//----------------------------------------------------------------------------
unsigned compute_crc32_table(unsigned char *data, int length, int *zerop)
{
    uint32_t crc = 0xFFFFFFFF;
    int i;
    for (i = 0; i < length; i++)
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc;
}

//----------------------------------------------------------------------------
// Function to compute CRC32 on a buffer
//----------------------------------------------------------------------------
unsigned compute_crc32_simul_table(unsigned char *data1, unsigned char * data2, int length, int *zerop)
{
    uint32_t crc1 = 0xFFFFFFFF;
    uint32_t crc2 = 0xFFFFFFFF;
	uint32_t crc3 = 0xFFFFFFFF;
	uint32_t crc4 = 0xFFFFFFFF;
	uint32_t crc5 = 0xFFFFFFFF;
	uint32_t crc6 = 0xFFFFFFFF;
	unsigned char * data3 = data2+8;
	unsigned char * data4 = data3+8;
	unsigned char * data5 = data4+8;
	unsigned char * data6 = data5+8;
    int i;

    for (i = 0; i < length; i++){
        crc1 = crc32_table[(crc1 ^ data1[i]) & 0xFF] ^ (crc1 >> 8);
        crc2 = crc32_table[(crc2 ^ data2[i]) & 0xFF] ^ (crc2 >> 8);
        crc3 = crc32_table[(crc3 ^ data3[i]) & 0xFF] ^ (crc3 >> 8);
        crc4 = crc32_table[(crc4 ^ data4[i]) & 0xFF] ^ (crc4 >> 8);
        crc5 = crc32_table[(crc5 ^ data5[i]) & 0xFF] ^ (crc5 >> 8);
        crc6 = crc32_table[(crc6 ^ data6[i]) & 0xFF] ^ (crc6 >> 8);
	}

	*zerop = crc2+crc3+crc4+crc5+crc6;
    return crc1;
}

//----------------------------------------------------------------------------
// Function to compute n CRCs at the same time.
//----------------------------------------------------------------------------
unsigned compute_crc32_simul_n(unsigned char *data[], int length, int num, uint32_t * crc_ret)
{
    uint32_t crc[20];
    int i,n;

    if (num > 20){
        printf("N too large %d\n",num);
        return 0;
    }

    memset(crc,0xff,sizeof(crc));

    for (i = 0; i < length; i++){
        for (n=0;n<num;n++){
            crc[n] = crc32_table[(crc[n] ^ data[n][i]) & 0xFF] ^ (crc[n] >> 8);
        }
	}

    // Its faster to use a local copy of CRC array than to just work with the
    // pointer passed in.  Probably compiler cautious due to aliasing rules.
    memcpy(crc_ret, crc, sizeof(int)*num);
    return 0;
}


//----------------------------------------------------------------------------
// Compute CRC32 , using if/else statement.
//----------------------------------------------------------------------------
unsigned compute_crc32_if_else_count(unsigned char *data, int length, int *zerop)
{
    uint32_t crc = 0xFFFFFFFF;
    const uint32_t polynomial = 0xEDB88320;

	int zeros = 0;
    int i,j;
    for (i = 0; i < length; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 1){
                crc = (crc >> 1) ^ polynomial;
			}else{
                crc >>= 1;
				zeros += 1;
			}
        }
    }
	*zerop = zeros;
    return crc;
}


//----------------------------------------------------------------------------
// Compute CRC32 , using if/else statement.
//----------------------------------------------------------------------------
unsigned compute_crc32_if_else(unsigned char *data, int length)
{
    uint32_t crc = 0xFFFFFFFF;
    const uint32_t polynomial = 0xEDB88320;

    int i,j;
    for (i = 0; i < length; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 1){
                crc = (crc >> 1) ^ polynomial;
			}else{
                crc >>= 1;
							}
        }
    }
    return crc;
}

//----------------------------------------------------------------------------
// Compute CRC32, always use same code path
//----------------------------------------------------------------------------

unsigned compute_crc32_and_xor(unsigned char *data, int length, int *zerop)
{
    uint32_t crc = 0xFFFFFFFF;
    const uint32_t polynomial = 0xEDB88320;

	int zeros = 0;
    int i,j;
    for (i = 0; i < length; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
			// Do it without branching, use boolean operations to xor with the
			// polynomial or zero depending on wheter last bit is set
			//zeros += (1-(crc & 1));
            crc = (crc >> 1) ^ ((0-(crc & 1)) & polynomial);
        }
    }
	*zerop = zeros;
    return crc;
}

