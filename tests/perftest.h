typedef unsigned int uint32_t;
typedef unsigned char uint8_t;


// pentominos.c
extern int PentominoBenchmark(void);

// 3d-pentomino.c
extern int Time3dPentominoSolver(void);

// crc_timing.c
extern void init_crc32_table(void);
extern unsigned compute_crc32_if_else(unsigned char *data, int length);
extern unsigned compute_crc32_if_else_count(unsigned char *data, int length, int *zerop);
extern unsigned compute_crc32_and_xor(unsigned char  *data, int length, int *zerop);
extern unsigned compute_crc32_table(unsigned char  *data, int length, int * zerop);

extern unsigned compute_simul_crc32_table(unsigned char *data, unsigned char * data2, int length, int *zerop);
extern unsigned compute_crc32_simul_n(unsigned char *datap[], int length, int num, uint32_t * crc_ret);

