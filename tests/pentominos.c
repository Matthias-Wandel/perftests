//--------------------------------------------------------------------------
// Program to figure out all the ways of placing the 12 pentominoes to 
// tile a 6x12 field, by recursively trying out all possible placements.
//
// Dec 30 2000 Matthias Wandel
//--------------------------------------------------------------------------
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include "perftest.h"

#define COMBINED_CHECK  // Makes it in 48 seconds on my Celeron 500

#define SHOW_SOLUTIONS 0

typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;
#define FALSE 0
#define TRUE 1
//#pragma pack(push,1)

typedef struct {
    BYTE Used[5][8];
}Piece_t;

char Desc[] = ".0123456789ABCDEF";

// Definition for the pieces.
BYTE Pc00[5][8] = {{3,3,3},{3},{3}};   // L
BYTE Pc01[5][8] = {{1,1},{1},{1,1}}; // c
BYTE Pc02[5][8] = {{2,2},{2,2},{2}}; // b
BYTE Pc03[5][8] = {{4,4},{4},{4},{4}}; // l
BYTE Pc04[5][8] = {{5},{5,5},{0,5},{0,5}}; // h
BYTE Pc05[5][8] = {{6},{6,6},{6},{6}}; // t
BYTE Pc06[5][8] = {{7,7,7},{0,7},{0,7}}; // T
BYTE Pc07[5][8] = {{0,8},{8,8,8},{0,8}}; // x
BYTE Pc08[5][8] = {{0,9},{9,9},{0,9,9}}; // r
BYTE Pc09[5][8] = {{10,10,10,10,10}};// I
BYTE Pc10[5][8] = {{0,11,11},{11,11},{11}};// w
BYTE Pc11[5][8] = {{12,12},{0,12},{0,12,12}};// z

Piece_t * Originals[] = {
    (Piece_t *) Pc00,
    (Piece_t *) Pc01,
    (Piece_t *) Pc02,
    (Piece_t *) Pc03,
    (Piece_t *) Pc04,
    (Piece_t *) Pc05,
    (Piece_t *) Pc06,
    (Piece_t *) Pc07,
    (Piece_t *) Pc08,
    (Piece_t *) Pc09,
    (Piece_t *) Pc10,
    (Piece_t *) Pc11,
};

// Each of 12 pieces has up to 8 varians for how it can be placed.
Piece_t Variants[12][8];
int NumVariants[12];   // Number of variants.

typedef struct {
    BYTE Pos[12][8];   // Grid for where the pieces go.
    BYTE Placed[12];   // These pieces are already placed.
}Field_t;

void TryPieces(Field_t * Field);


//--------------------------------------------------------------------------
// Show a piece.
//--------------------------------------------------------------------------
void ShowPiece(Piece_t * Piece)
{
    int x,y,a;
    for (y=5;;){
        if (!y--) break;
        for (x=0;x<5;x++){
            a = Piece->Used[y][x];
            if (a == 0){
                printf(".");
            }else{
                printf("%c",Desc[a & 15]);
            }
        }
        printf("\n");
    }
    printf("\n");
}
//--------------------------------------------------------------------------
// Show a piece.
//--------------------------------------------------------------------------
void ShowField(Field_t * Field)
{
    int x,y,a;
    for (y=12;;){
        if (!y--) break;
        for (x=0;x<8;x++){
            a = Field->Pos[y][x];
            if (a == 0){
                printf(".");
            }else{
                if (a == 0xff){
                    printf("+");
                }else{
                    printf("%c",Desc[a & 15]);
                }
            }
        }
        printf("\n");
    }
}

//--------------------------------------------------------------------------
// Show the field fancy.
//--------------------------------------------------------------------------
void ShowFancy(Field_t * Field)
{
    int row,col;
    int r;

    for (row=10;row>0;row--){

        for (r=0;r<2;r++){ // Print the same stuff 3 times for clarity...
            for (col=1;col<=6;col++){
                if (Field->Pos[row][col]){
                    printf("####");
                }else{
                    printf("    ");
                }
                if (col < 6){
                    if (Field->Pos[row][col] == Field->Pos[row][col+1]){
                        if (Field->Pos[row][col]){
                            printf("#");
                        }else{
                            printf(":");
                        }
                    }else{
                        printf(" ");
                    }
                }
            }
            printf ("\n");
        }

        if (row > 1){
            for (col=1;col<=6;col++){
                if (Field->Pos[row][col] == Field->Pos[row-1][col]){
                    if (Field->Pos[row][col]){
                        printf("####");

                    }else{
                        printf("----");
                    }
                }else{
                    printf("    ");
                }
                printf(" ");
            }
            printf("\n");
        }
        
    }

}




//--------------------------------------------------------------------------
// Copy and rotate a piece ccw.
//--------------------------------------------------------------------------
void RotatePiece(Piece_t * Dest, Piece_t * Src)
{
    int x,xput,y,Flag;
    memset(Dest, 0, sizeof(Piece_t));
    Flag=FALSE;
    xput = 0;
     
    for (x=5;;){
        if (!x--) break;
        for (y=0;y<5;y++){
            Dest->Used[xput][y] = Src->Used[y][x];
            if (Src->Used[y][x]) Flag=TRUE;
        }
        if(Flag) xput++;
    }
}

//--------------------------------------------------------------------------
// Copy and mirror image a piece.
//--------------------------------------------------------------------------
void FlipPiece(Piece_t * Dest, Piece_t * Src)
{
    int x,xput,y,Flag;
    memset(Dest, 0, sizeof(Piece_t));
    Flag=FALSE;
    xput = 0;
     
    for (x=5;;){
        if (!x--) break;
        for (y=0;y<5;y++){
            Dest->Used[y][xput] = Src->Used[y][x];
            if (Src->Used[y][x]) Flag=TRUE;
        }
        if(Flag) xput++;
    }
}



int GlobalPlaces = 0;
int GlobalAlmost = 0;
int GlobalSolutions = 0;
//--------------------------------------------------------------------------
// The recursive searching function...
//--------------------------------------------------------------------------
void TryPieces(Field_t * Pre)
{
    int pn;
    int BottomFreeRow = 0;
    int BottomFreeCol = 0;
    int NumPlaced;
    Field_t Field;

    // Establish left most position in bottom most free row with a free
    // square.  The next piece MUST occupy this - cuts down on duplications.
    {
        int row,col;
        for (row=1;row<=12;row++){
            for (col=1;col<=6;col++){
                if (Pre->Pos[row][col] == 0){
                    BottomFreeRow = row;
                    BottomFreeCol = col;
                    goto done;
                }
            }
        }
    }
    done:

    // Count pieces placed so far (need that later)
    NumPlaced = 0;
    for (pn=0;pn<12;pn++){
        if (Pre->Placed[pn]) NumPlaced += 1;
    }

    // Copy field for messing about with.
    Field = *Pre;

    // Now try each piece in each position.

    // Loop for peice number
    for (pn=0;pn<12;pn++){
        int pv;

        // Check if this piece is already placed.
        if (Pre->Placed[pn]) continue;

        // Loop for piece orientation.
        for (pv=0;pv<NumVariants[pn];pv++){
            Piece_t TryPiece;
            int px,py;
            TryPiece = Variants[pn][pv];

            py = BottomFreeRow; // Piece must occupy bottom-most position.

            // Loop for piece X (piece Y must be bottom most free)
            for (px=1;px<=6;px++){
                if (px > BottomFreeCol){
                    // Too far to the right.  Could not possibly occupy the spot.
                    break;
                }

#ifdef COMBINED_CHECK
//#define COMBINED_IF
#ifdef COMBINED_IF
                    // Combining the first two checks makes it run about 14% faster!
                    // Slightly more speedup by cominging the first 3 conditions, but slower
                    // on 2008 and prior intel CPUs
                    if (  (*((DWORD *)&(TryPiece.Used[0][0]))& *((DWORD *)&(Field.Pos[py][px])))
                        |
                          (*((DWORD *)&(TryPiece.Used[1][0]))& *((DWORD *)&(Field.Pos[py+1][px])))
                        ){
                        goto pos_failed;
                    }
#else
                    if (   *((DWORD *)&(TryPiece.Used[0][0]))
                         & *((DWORD *)&(Field.Pos[py][px]))
                        ){
                        goto pos_failed;
                    }

                    if (   *((DWORD *)&(TryPiece.Used[1][0]))
                         & *((DWORD *)&(Field.Pos[py+1][px]))){
                        goto pos_failed;
                    }
#endif

                    if (   *((DWORD *)&(TryPiece.Used[2][0]))
                         & *((DWORD *)&(Field.Pos[py+2][px]))){
                        goto pos_failed;
                    }

                    if (   *((WORD *)&(TryPiece.Used[3][0]))
                         & *((WORD *)&(Field.Pos[py+3][px]))){
                        goto pos_failed;
                    }

                    if (TryPiece.Used[0][4] & Field.Pos[py][px+4] ){
                        goto pos_failed;
                    }

                    if (TryPiece.Used[4][0] & Field.Pos[py+4][px] ){
                        goto pos_failed;
                    }

                    if (TryPiece.Used[0][BottomFreeCol-px] == 0) continue; // does not hit the desired spot.


                    *((DWORD *)&(Field.Pos[py+0][px])) |= *((DWORD *)&(TryPiece.Used[0][0]));
                                 Field.Pos[py+0][px+4] |= TryPiece.Used[0][4];
                    *((DWORD *)&(Field.Pos[py+1][px])) |= *((DWORD *)&(TryPiece.Used[1][0]));
                    *((DWORD *)&(Field.Pos[py+2][px])) |= *((DWORD *)&(TryPiece.Used[2][0]));
                    *((DWORD *)&(Field.Pos[py+3][px])) |= *((DWORD *)&(TryPiece.Used[3][0]));
                                 Field.Pos[py+4][px]   |= TryPiece.Used[4][0];

#else

                    // Loop for row and column trying to place it.
                    for (row=0;row<5;row++){
                        for(col=0;col<5;col++){
                            if (TryPiece.Used[row][col] & Field.Pos[row+py][col+px]){
                                // There is a conflict.  Position fails.
                                goto pos_failed;
                            }
                        }
                    }

                    // Piece pn variant pv fits at px,py

                    // Place the piece.
                    for (row=0;row<5;row++){
                        for(col=0;col<5;col++){
                            Field.Pos[row+py][col+px] |= TryPiece.Used[row][col];
                        }
                    }
#endif


                Field.Placed[pn] = 1;


                if (Field.Pos[BottomFreeRow][BottomFreeCol] == 0){
                    // The bottom left free sqare was NOT occupied by the piece.
                    goto redundant_pos;
                }


                GlobalPlaces += 1;

                if (NumPlaced+1 >= 11){
                    GlobalAlmost += 1;
                }

                if (GlobalSolutions > 5000) return;
                if (NumPlaced+1 >= 12){
                    #ifndef TEST_MODULE
						printf("\nSol %4d Almost %8d Placings %8d\n",
                                GlobalSolutions, GlobalAlmost,GlobalPlaces);
						//ShowField(&Field);
						ShowFancy(&Field);
					#endif
                    GlobalSolutions += 1;
                }

                TryPieces(&Field);


                // Un-place the piece for the next try...
                redundant_pos:
                Field = *Pre;

                pos_failed:;
            }
        }
    }
}


//--------------------------------------------------------------------------
// Run the pentomino solver for benchmarking purposes.
//--------------------------------------------------------------------------
int PentominoBenchmark(void)
{
    int a,b,c;
    Piece_t Temp1, Temp2;
	GlobalPlaces = 0;
	GlobalAlmost = 0;
	GlobalSolutions = 0;

    /*
    for (a=0;a<12;a++){
        printf("Piece %d\n",a);
        ShowPiece(Originals[a]);
    }
    */

    //--------------------------------------------------------------
    // Algorithm to generate unique piece variants.
    for (b=0;b<12;b++){
        int VariantCount;
        Temp1 = *Originals[b];

        for (a=0;a<5;a++){
            for (c=0;c<5;c++){
                if (Temp1.Used[a][c]){
                    Temp1.Used[a][c] |= 0x10;
                }
            }
        }

        VariantCount = 0;
        for (a=0;a<8;a++){

            for (c=0;c<VariantCount;c++){
                if (memcmp(&Variants[b][c], &Temp1, sizeof(Piece_t)) == 0){
                    goto nostore;
                }
            }

            Variants[b][VariantCount++] = Temp1;
            nostore:

            // Rotate the piece.
            Temp2 = Temp1;
            RotatePiece(&Temp1, &Temp2);

            // After 4 tries, flip the piece.
            if (a == 3){
                Temp2 = Temp1;
                FlipPiece(&Temp1, &Temp2);
            }
        }
        NumVariants[b] = VariantCount;
    }

    //-------------------------------------------------------------
    // Show the variants.
    /*
    printf("\nVariants:\n");

    for (b=0;b<12;b++){
        for (a=0;a<NumVariants[b];a++){
            printf("piece %d variant %d\n",b,a);
            ShowPiece(&Variants[b][a]);
        }
    }
    */

    // By limiting the variants of the diagonally symetric L piece,
    // we ensure we don't get different ways of orienting the whole puzzle.
    NumVariants[0] = 1;

    {
        Field_t Field;
        memset(&Field, 0, sizeof(Field_t));

        // Initialize the playing field.
        for (a=0;a<12;a++){
            Field.Pos[a][0] = 0xff;
            Field.Pos[a][7] = 0xff;
        }
        for (a=0;a<8;a++){
            Field.Pos[0][a] = 0xff;
            Field.Pos[11][a] = 0xff;
        }

        //ShowFancy(&Field);
        TryPieces(&Field);
    }
	if (GlobalSolutions != 2339) printf("Solutins found: %d\n",GlobalSolutions);

    return GlobalSolutions;
}


#ifndef TEST_MODULE
//--------------------------------------------------------------------------
// Mainline
//--------------------------------------------------------------------------
int main(void)
{
	time_t start,end;
	
	time(&start);
	int NumSolutions = PentominoBenchmark();
    time(&end);

    printf("\nElapsed time:%d\n",(int)(end-start));
	
	printf("Solutins found: %d\n",NumSolutions);
	return 0;
}
#endif