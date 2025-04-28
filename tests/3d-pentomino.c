//-------------------------------------------------------------------------------
// 3D pentomino solver program.
//
// Matthias Wandel.
//
// Originally writte: Fall 2002
// Some refinement for use in my perftests program April 2025
//-------------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

// Always make X size the largest for faster solving.
// A map of x*y*z = 10*5*3 solves fastest
#define FIELD_X_SIZE 6
#define FIELD_Y_SIZE 5
#define FIELD_Z_SIZE 5

//#define INCLUDE_HEXOMINOS 1
typedef struct {
    struct {
        char x,y,z;
    }cubes[6];
}PieceList_t;


#ifdef INCLUDE_HEXOMINOS
    #define   NUM_PIECES  (31+29)
#else
    #define   NUM_PIECES  (31)
#endif

PieceList_t Pieces[NUM_PIECES] = {
#ifdef INCLUDE_HEXOMINOS
    // Some hexominos
    {{{0,0,0}, {1,0,0}, {0,0,1}, {0,0,2}, {0,0,3}, {0,1,3}}},// Long twisted hook
    {{{0,0,0}, {0,1,0}, {0,0,1}, {0,0,2}, {0,0,3}, {1,0,3}}},// Long twisted hook mirror

    {{{0,0,0}, {1,0,0}, {1,1,0}, {1,2,0}, {1,2,1}, {2,2,1}}},// offset S
    {{{0,0,1}, {1,0,1}, {1,1,1}, {1,2,1}, {1,2,0}, {2,2,0}}},// Offset S mirror

    {{{0,0,0}, {1,0,0}, {2,0,0}, {0,0,1}, {0,0,2}, {0,1,2}}},// L with side hook
    {{{0,0,0}, {1,0,0}, {2,0,0}, {0,0,1}, {0,0,2}, {2,1,0}}},// L with side hook mirror

    {{{0,0,0}, {1,0,0}, {2,0,0}, {0,0,1}, {0,0,2}, {0,1,1}}},// Long arm twisted R
    {{{0,0,0}, {1,0,0}, {2,0,0}, {0,0,1}, {0,0,2}, {1,1,0}}},// Long arm twisted R mirror

    {{{0,0,0}, {0,1,0}, {0,2,0}, {0,3,0}, {1,3,1}, {0,3,1}}},// Twisted hook long
    {{{0,0,0}, {0,1,0}, {0,2,0}, {0,3,0}, {1,3,1}, {1,3,0}}},// Twisted hook long mirror

    {{{2,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, {0,1,1}, {0,2,1}}},// Mutant W
    {{{2,0,1}, {1,0,1}, {1,1,1}, {0,1,1}, {0,1,0}, {0,2,0}}},// Mutant W mirror

    {{{0,0,0}, {1,0,0}, {2,0,0}, {0,1,0}, {0,1,1}, {0,1,2}}},// Offset L
    {{{0,0,0}, {1,0,0}, {2,0,0}, {0,0,1}, {0,1,1}, {0,2,1}}},// Offset L mirror

    {{{0,0,0}, {1,0,0}, {1,1,0}, {1,1,1}, {1,2,0}, {2,2,0}}},// S with side tab
    {{{0,0,0}, {0,1,0}, {1,1,0}, {1,1,1}, {2,1,0}, {2,2,0}}},// S with side tab mirror

    {{{0,0,0}, {0,0,1}, {0,1,0}, {1,1,0}, {1,2,0}, {1,2,1}}},// small s with tabs
    {{{0,0,0}, {0,0,1}, {1,0,0}, {1,1,0}, {2,1,0}, {2,1,1}}},// small s with tabs mirror

    // symetrical hexominos
    {{{0,1,0}, {1,1,0}, {2,1,0}, {1,0,1}, {1,1,1}, {1,2,1}}},// Crossing 3's
    {{{0,0,0}, {1,0,0}, {2,0,0}, {1,1,0}, {1,2,0}, {1,2,1}}},// T with hooked stem
    {{{0,0,0}, {1,0,0}, {2,0,0}, {1,1,0}, {1,1,1}, {1,2,1}}},// T with folded stem

    // Flat hexominos
    {{{0,0,0}, {0,1,0}, {0,1,1}, {0,1,2}, {0,1,3}, {0,2,3}}},// s
    {{{0,0,0}, {1,0,0}, {0,0,1}, {0,0,2}, {0,0,3}, {0,0,4}}},// bench dog
    {{{0,0,0}, {1,0,0}, {2,0,0}, {0,0,1}, {0,0,2}, {0,0,3}}},// L
    {{{0,0,0}, {1,0,0}, {1,1,0}, {1,2,0}, {2,2,0}, {1,3,0}}},// Stretched r
    {{{0,0,0}, {1,0,0}, {0,1,0}, {0,2,0}, {0,3,0}, {1,3,0}}},// big C
    {{{0,0,0}, {0,1,0}, {1,1,0}, {1,2,0}, {1,3,0}, {2,3,0}}},// Mutant W
    {{{0,0,0}, {0,0,1}, {0,1,1}, {0,0,2}, {0,0,3}, {0,0,4}}},// Five with a tab.
    {{{0,0,0}, {1,0,0}, {2,0,0}, {2,1,0}, {2,2,0}, {3,2,0}}},// L with hook
#endif

//---------------------------------------------------------

    // The twelve flat pentominos.
    {{{1,0,0}, {0,0,0}, {0,1,0}, {0,2,0}, {0,3,0}}},  // i
    {{{0,0,0}, {1,0,0}, {2,0,0}, {3,0,0}, {4,0,0}}},  // I
    {{{0,2,0}, {0,1,0}, {0,0,0}, {1,0,0}, {1,1,0}}},  // b
    {{{2,0,0}, {1,0,0}, {0,0,0}, {0,1,0}, {0,2,0}}},  // L
    {{{2,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, {0,2,0}}},  // W
    {{{0,0,0}, {1,0,0}, {2,0,0}, {2,1,0}, {3,1,0}}},  // z
    {{{1,0,0}, {0,0,0}, {0,1,0}, {0,2,0}, {1,2,0}}},  // u
    {{{0,0,0}, {1,0,0}, {2,0,0}, {3,0,0}, {1,1,0}}},  // t
    {{{0,0,0}, {1,0,0}, {2,0,0}, {1,1,0}, {1,2,0}}},  // T
    {{{1,0,0}, {1,1,0}, {0,1,0}, {1,2,0}, {2,1,0}}},  // x
    {{{1,0,0}, {1,1,0}, {0,1,0}, {1,2,0}, {2,2,0}}},  // r
    {{{0,0,0}, {1,0,0}, {1,1,0}, {1,2,0}, {2,2,0}}},  // s
    // The 17 nonflat pentominos
    // Symetrical:
    {{{0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, {0,0,1}}},  // Square with a block on it. (symetrical)
    {{{0,0,0}, {1,0,0}, {0,1,0}, {1,0,1}, {0,1,1}}},  // Compact diagonal
    {{{0,0,0}, {1,0,0}, {0,1,0}, {0,0,1}, {0,0,2}}},  // Bent Y shape. (symetrical)
    {{{0,0,0}, {0,0,1}, {0,0,2}, {0,1,1}, {1,0,1}}},  // Octopus
    {{{0,0,0}, {0,0,1}, {0,0,2}, {0,1,1}, {1,1,1}}},  // Elephant

    // Not symetrical:
    {{{1,0,0}, {1,1,0}, {1,2,0}, {0,2,0}, {1,1,1}}},  // Twisted R pentomino
    {{{1,0,0}, {1,1,0}, {1,2,0}, {2,2,0}, {1,1,1}}},  // Twisted R pentomino mirror
    {{{0,0,0}, {0,1,0}, {0,2,0}, {1,2,1}, {0,2,1}}},  // Twisted hook
    {{{0,0,0}, {0,1,0}, {0,2,0}, {1,2,1}, {1,2,0}}},  // Twisted hook mirror
    {{{0,0,0}, {0,1,0}, {0,2,0}, {1,0,0}, {0,2,1}}},  // Twisted s
    {{{0,0,0}, {0,1,0}, {0,2,0}, {0,0,1}, {1,2,0}}},  // Twistes s mirror
    {{{0,0,0}, {0,0,1}, {0,1,1}, {1,1,1}, {1,1,2}}},  // Folded s
    {{{0,0,0}, {0,0,1}, {1,0,1}, {1,1,1}, {1,1,2}}},  // Folded s mirror
    {{{0,0,0}, {0,1,0}, {0,0,1}, {1,0,1}, {1,0,2}}},  // small s with side cube
    {{{0,0,0}, {1,0,0}, {0,0,1}, {0,1,1}, {0,1,2}}},
    {{{0,0,0}, {0,1,1}, {0,0,1}, {1,0,1}, {1,0,2}}},  // small s with side cube on end
    {{{0,0,0}, {1,0,1}, {0,0,1}, {0,1,1}, {0,1,2}}},

    // Some extra cubes to make the number more divisible.
//    {{{{0,0,0}, {1,0,0}, {1,1,0}, {1,1,1}, {0,0,0}}}},
//    {{{{0,0,0}, {1,0,0}, {2,0,0}, {3,0,0}, {0,0,0}}}},
    {{{0,0,0}, {1,0,0}, {2,0,0}, {0,0,0}, {0,0,0}}},
    {{{0,0,0}, {1,0,0}, {0,0,0}, {0,0,0}, {0,0,0}}}

};






// Display cube size.
#define CU_WIDE  5
#define CU_TALL  4
#define CU_DEPTH 2

// Display text map size. 
#define GRIDWIDTH 150
#define GRIDHEIGHT 60

#define TRUE 1
#define FALSE 0
typedef int BOOL;

//  Y   Z
//
//  | /
//  |/
//  +----> X

static char CharGraph[GRIDHEIGHT][GRIDWIDTH];

#define OMIT_LEFT_FRONT_EDGE 1
#define OMIT_LEFT_TOP_EDGE 2
#define OMIT_BOTTOM_FRONT_EDGE 4
#define OMIT_BOTTOM_RIGHT_EDGE 8
#define OMIT_BACK_TOP_EDGE 16
#define OMIT_BACK_RIGHT_EDGE 32

#define CUBENUM_HILIGHT (-100)

typedef int bool;
#define TRUE 1
#define FALSE 0

//-------------------------------------------------------------------------------
// Add a cube to the display buffer.
//-------------------------------------------------------------------------------
void PutCube(int x,int y,int z, int Omit, int CubeNum)
{
    int across,up;
    int a,b;
    char Str[20];
    char SpaceChar;
    
    SpaceChar = ' ';

    across = x*CU_WIDE+z*CU_DEPTH;
    up = y*CU_TALL+z*CU_DEPTH;
    if (CubeNum >= 0){
        sprintf(Str, "%d",CubeNum);
    }else{
        Str[0] = '\0';
        if (CubeNum == CUBENUM_HILIGHT){
            SpaceChar = '\\';
        }
    }

    // Draw the corners of the front face.
    CharGraph[up        ][across        ] = '+';
    CharGraph[up        ][across+CU_WIDE] = '+';
    CharGraph[up+CU_TALL][across+CU_WIDE] = '+';
    CharGraph[up+CU_TALL][across        ] = '+';

    // Draw the back corners.
    CharGraph[up+CU_DEPTH][        across+CU_DEPTH+CU_WIDE] = '+';
    CharGraph[up+CU_DEPTH+CU_TALL][across+CU_DEPTH+CU_WIDE] = '+';
    CharGraph[up+CU_DEPTH+CU_TALL][across+CU_DEPTH        ] = '+';


    // Draw the horizontal lines.
    for (a=1;a<CU_WIDE;a++){
        if (Omit & OMIT_BOTTOM_FRONT_EDGE){
            CharGraph[up][across+a] = SpaceChar;
        }else{
            CharGraph[up][across+a] = '-';
        }
        CharGraph[up+CU_TALL][across+a] = '-';
        if (Omit & OMIT_BACK_TOP_EDGE){
            CharGraph[up+CU_TALL+CU_DEPTH][ across+CU_DEPTH+a] = SpaceChar;
        }else{
            CharGraph[up+CU_TALL+CU_DEPTH][ across+CU_DEPTH+a] = '-';
        }
    }

    // Draw the vertical lines.
    for (a=1;a<CU_TALL;a++){
        if (Omit & OMIT_LEFT_FRONT_EDGE){
            CharGraph[up+a][across] = SpaceChar;
        }else{
            CharGraph[up+a][across] = '|';
        }
        CharGraph[up+a][across+CU_WIDE] = '|';
        if (Omit & OMIT_BACK_RIGHT_EDGE){
            CharGraph[up+a+CU_DEPTH][ across+CU_WIDE+CU_DEPTH] = SpaceChar;
        }else{
            CharGraph[up+a+CU_DEPTH][ across+CU_WIDE+CU_DEPTH] = '|';
        }
    }

    // Draw the lines extending back.
    for (a=1;a<CU_DEPTH;a++){
        if (Omit & OMIT_LEFT_TOP_EDGE){
            CharGraph[up+CU_TALL+a][across+a] = SpaceChar;
        }else{
            CharGraph[up+CU_TALL+a][across+a] = '/';
        }
        CharGraph[up+CU_TALL+a][across+CU_WIDE+a] = '/';
        if (Omit & OMIT_BOTTOM_RIGHT_EDGE){
            CharGraph[up+a][across+CU_WIDE+a] = SpaceChar;
        }else{
            CharGraph[up+a][across+CU_WIDE+a] = '/';
        }
    }

    for (a=1;a<CU_TALL;a++){
        // Clear the front face.
        for (b=1;b<CU_WIDE;b++){
            CharGraph[up+a][across+b] = SpaceChar;
        }
        // Clear the right face.
        for (b=1;b<CU_DEPTH;b++){
            CharGraph[up+a+b][across+CU_WIDE+b] = SpaceChar;
        }
    }
    // Clear up top face.
    for (a=1;a<CU_DEPTH;a++){
        for (b=1;b<CU_WIDE;b++){ 
            CharGraph[up+CU_TALL+a][across+a+b] = SpaceChar;
        }
    }

    // Label the piece.
    for (a=0;a<5;a++){
        if (Str[a] == 0) break;
        CharGraph[up+CU_TALL/2][across+2+a] = Str[a];
        CharGraph[up+CU_TALL+1][across+3+a] = Str[a];
    }

}

//-------------------------------------------------------------------------------
// Initialize the display buffer.
//-------------------------------------------------------------------------------
void InitGraph(void)
{
    memset(&CharGraph, ' ', sizeof(CharGraph));
}


//-------------------------------------------------------------------------------
// Print the display buffer
//-------------------------------------------------------------------------------
void PrintGraph(void)
{
    int a,b;
    for (a=GRIDHEIGHT;;){
        a--;
        CharGraph[a][GRIDWIDTH-1] = '\0';
        for (b=GRIDWIDTH-1;;){
            b--;
            if (CharGraph[a][b] != ' '){
                break;
            }
            if (b == 0) goto no_line;
        }
        CharGraph[a][b+1] = '\0';
        puts(CharGraph[a]);
        no_line:
        if (a == 0) break;
    }
}


//###############################################################################
typedef struct {
    int x_incr;
    int y_incr;
    int z_incr;
    int x_max;
    int y_max;
    int z_max;
    char * Data;
}Map_3d_t;


void Show3dMap(Map_3d_t Piece, BOOL ShowNums, int HilightedCube)
{
    int x,y,z;
    InitGraph();
    for (z=Piece.z_max;;){
        z -= 1;
        for (y=0;y<Piece.y_max;y++){
            for (x=0;x<Piece.x_max;x++){
                int v;
                v = Piece.Data[x*Piece.x_incr+y*Piece.y_incr+z*Piece.z_incr];
                if (v){
                    int Omit = 0;

                    if (y > 0 && (Piece.Data[x*Piece.x_incr+(y-1)*Piece.y_incr+z*Piece.z_incr] == v)){
                        Omit |= OMIT_BOTTOM_FRONT_EDGE;
                        if (x >= (Piece.x_max-1) || (Piece.Data[(x+1)*Piece.x_incr+(y-1)*Piece.y_incr+z*Piece.z_incr] == 0)){
                            Omit |= OMIT_BOTTOM_RIGHT_EDGE; 
                        }
                    }
                    if (x > 0 && (Piece.Data[(x-1)*Piece.x_incr+y*Piece.y_incr+z*Piece.z_incr] == v)){
                        Omit |= OMIT_LEFT_FRONT_EDGE;
                        Omit |= OMIT_LEFT_TOP_EDGE;
                    }
                    if (z < (Piece.z_max-1) && (Piece.Data[x*Piece.x_incr+y*Piece.y_incr+(z+1)*Piece.z_incr] == v)){
                        if (x >= (Piece.x_max-1) || (Piece.Data[(x+1)*Piece.x_incr+y*Piece.y_incr+(z+1)*Piece.z_incr] == 0)){
                            Omit |= OMIT_BACK_RIGHT_EDGE;
                        }
                        if (y >= (Piece.y_max-1) || (Piece.Data[x*Piece.x_incr+(y+1)*Piece.y_incr+(z+1)*Piece.z_incr] == 0)){
                            Omit |= OMIT_BACK_TOP_EDGE;
                        }
                    }
                    //printf("cube at (%d,%d,%d)\n",x,y,z);
                    if (ShowNums){
                        if (v != HilightedCube){
                            PutCube(x, y, z, Omit, v-1);
                        }else{
                            PutCube(x, y, z, Omit, CUBENUM_HILIGHT);
                        }
                    }else{
                        PutCube(x, y, z, Omit, -1);
                    }
                }
            }
        }
        if (z == 0) break;
    }
    PrintGraph();
}


//###############################################################################

typedef struct {
    char Data[5][5][5];
}Map_t;

//-------------------------------------------------------------------------------
// Generate pice map from predefined stuff.
//-------------------------------------------------------------------------------
void ReadPiece(Map_t * Piece, int Index)
{
    int a;
    memset(Piece->Data, 0, 5*5*5);
    for (a=0;a<6;a++){
        int x,y,z;
        x = Pieces[Index].cubes[a].x;
        y = Pieces[Index].cubes[a].y;
        z = Pieces[Index].cubes[a].z;
        if (a < 5 || x || y || z){
            Piece->Data[x][y][z] = -1;
        }
    }
}


//-------------------------------------------------------------------------------
// rotate about X axis.
//-------------------------------------------------------------------------------
void Rotate_X(Map_t * Piece)
{
    char Map[5][5][5];
    int x,y,z;
    memcpy(Map, Piece->Data, 5*5*5);
    for (x=0;x<5;x++){
        for (y=0;y<5;y++){
            for (z=0;z<5;z++){
                Piece->Data[x][y][z] = Map[x][4-z][y];
            }
        }
    }
}


//-------------------------------------------------------------------------------
// rotate about Y axis.
//-------------------------------------------------------------------------------
void Rotate_Y(Map_t * Piece)
{
    char Map[5][5][5];
    int x,y,z;
    memcpy(Map, Piece->Data, 5*5*5);
    for (x=0;x<5;x++){
        for (y=0;y<5;y++){
            for (z=0;z<5;z++){
                Piece->Data[x][y][z] = Map[z][y][4-x];
            }
        }
    }
}
//-------------------------------------------------------------------------------
// rotate about Z axis.
//-------------------------------------------------------------------------------
void Rotate_Z(Map_t * Piece)
{
    char Map[5][5][5];
    int x,y,z;
    memcpy(Map, Piece->Data, 5*5*5);
    for (x=0;x<5;x++){
        for (y=0;y<5;y++){
            for (z=0;z<5;z++){
                Piece->Data[x][y][z] = Map[y][4-x][z];
            }
        }
    }
}

//-------------------------------------------------------------------------------
// Minimize x y and z vlaues if possible.
//-------------------------------------------------------------------------------
void MinimizeValues(Map_t * Piece)
{
    char Map[5][5][5];
    int x,y,z;
    int mx,my,mz;
    memcpy(Map, Piece->Data, 5*5*5);

    mx = my = mz = 4;
    for (x=0;x<5;x++){
        for (y=0;y<5;y++){
            for (z=0;z<5;z++){
                if (Piece->Data[x][y][z]){
                    if (mx > x) mx = x;
                    if (my > y) my = y;
                    if (mz > z) mz = z;
                }
            }
        }
    }

    memset(Piece, 0, 5*5*5);
    for (x=0;x<5-mx;x++){
        for (y=0;y<5-my;y++){
            for (z=0;z<5-mz;z++){
                Piece->Data[x][y][z] = Map[x+mx][y+my][z+mz];
            }
        }
    }
}

//-------------------------------------------------------------------------------
// Show a 5x5x5 piece in the given orientation.
//-------------------------------------------------------------------------------
void Show5(Map_t * Map)
{
    Map_3d_t Piece;

    Piece.Data = (char *)Map;
    Piece.x_incr = 25;
    Piece.y_incr = 5;
    Piece.z_incr = 1;
    Piece.x_max = 5;
    Piece.y_max = 5;
    Piece.z_max = 5;
    Show3dMap(Piece, FALSE, -1);
}


//-------------------------------------------------------------------------------
// Show a 5x5x5 piece, front and back.
//-------------------------------------------------------------------------------
void Show5_FrontAndBack(Map_t * Map)
{
    struct {
        char Data[11][5][5];
    }Copy;
    Map_3d_t Piece;
    int x,y,z;

    memset(&Copy, 0, sizeof(Copy));

    // Copy the piece into the larger map, in two orientation.
    for (x=0;x<5;x++){
        for (y=0;y<5;y++){
            for (z=0;z<5;z++){
                Copy.Data[x][y][z] = Map->Data[x][y][z]; 
                Copy.Data[x+6][y][z] = Map->Data[4-z][y][x]; 
            }
        }
    }

    // Put the second orientation close to home position.
    MinimizeValues((Map_t *) Copy.Data[6]);

    Piece.Data = (char *)Copy.Data;
    Piece.x_incr = 25;
    Piece.y_incr = 5;
    Piece.z_incr = 1;
    Piece.x_max = 11;
    Piece.y_max = 5;
    Piece.z_max = 5;

    Show3dMap(Piece, FALSE, -1);
}

//###############################################################################

//-------------------------------------------------------------------------------
// Generate the 24 possible orientations, eliminating duplicates.
//-------------------------------------------------------------------------------
int UniqueOrientations(Map_t * Ors, Map_t Piece)
{
    int a,b, NumUnique;
    Ors[0] = Piece;

    Ors[4] = Piece;
    Rotate_X(Ors+4);

    Ors[8] = Piece;
    Rotate_X(Ors+8);
    Rotate_X(Ors+8);
    Rotate_X(Ors+8);

    Ors[12] = Piece;
    Rotate_Y(Ors+12);

    Ors[16] = Piece;
    Rotate_Y(Ors+16);
    Rotate_Y(Ors+16);
    Rotate_Y(Ors+16);

    Ors[20] = Piece;
    Rotate_Y(Ors+20);
    Rotate_Y(Ors+20);

    for (a=0;a<24;a+= 4){
        Ors[a+1] = Ors[a];
        Rotate_Z(&Ors[a+1]);
        Ors[a+2] = Ors[a+1];
        Rotate_Z(&Ors[a+2]);
        Ors[a+3] = Ors[a+2];
        Rotate_Z(&Ors[a+3]);
    }
    for (a=0;a<24;a++){
        MinimizeValues(Ors+a);
    }

    NumUnique = 0;

    // Now eliminate identical orientations.
    for (a=0;a<24;a++){
        for (b=0;b<NumUnique;b++){
            if (memcmp(&Ors[a], &Ors[b], sizeof(Map_t)) == 0){
                //Show5(Ors+NumUnique);
                //printf("Or %2d == %2d\n",a,b);
                goto not_unique;
                break;
            }
        }
        Ors[NumUnique] = Ors[a];
        //printf("Orientation %d (%d)\n",NumUnique,a);
        //Show5(Ors+NumUnique);
        NumUnique += 1;
        not_unique:;
    }
    return NumUnique;
}

//###############################################################################

// Use a 32 bit word with each bit representing a relative position where 
// interference is likely.  By and-ing maps for a fit pre-check, fittings are greatly accelerated.
struct {
    char x,y,z;
} FitMap[32] = { // dx, dy, dz
    // On the same plane.  -z and -y already filled, 
    {0,0,1}, { 0,0,2},  {0,0,3},
    {0,1,-2}, {0,1,-1}, {0,1,0}, {0,1,1}, {0,1,2},
    {0,2,-1}, {0,2,0},  {0,2,1},
    {0,3,0},
    // Next plane over
    {1,-2,0},
    {1,-1,-1}, {1,-1,0}, {1,-1,1},
    {1,0,-2},  {1,0,-1}, {1,0,0}, {1,0,1}, {1,0,2},
    {1,1,-1},  {1,1,0},  {1,1,1},
    {1,2,0},
    
    // Next plane over
    {2,-1,0},
    {2,0,-1}, {2,0,0}, {2,0,1},
    {2,1,0},

    // Last one over.
    {3,0,0},

    {4,0,0}

};

typedef struct {
    Map_t Orientations[24];
    int NumOrientations;

    struct {
        // Smallest Y and Z of first cube (that we try to fill)
        int YOffset, ZOffset;
         unsigned long FitWord;
    }FitOpt[24];
}PieceData_t;

PieceData_t AllPieces[NUM_PIECES];

int PlacementsTried = 0;

//-------------------------------------------------------------------------------
// Prepare 3d representations of all possible pieces in all orientations.
//-------------------------------------------------------------------------------
void PreparePieces(void)
{
    int a,b,c;
    int my,mz;

    for (a=0;a<NUM_PIECES;a++){
        Map_t Map;
        ReadPiece(&Map, a);

        // Make the unique orientations
        AllPieces[a].NumOrientations = 
            UniqueOrientations(AllPieces[a].Orientations, Map);

        for (b=0;b<AllPieces[a].NumOrientations;b++){
            // For each orientation, find the first ocupied cube in y and z.  This is the cube
            // that we will try to put into the next available position when solving.
            unsigned long FitWord;
            Map = AllPieces[a].Orientations[b];

            for (my=0;my<5;my++){
                for (mz=0;mz<5;mz++){
                    if (Map.Data[0][my][mz]) goto found;
                }
            }
            printf("Error! Empty piece\n");
            exit(-1);
            found:
            AllPieces[a].FitOpt[b].YOffset = my;
            AllPieces[a].FitOpt[b].ZOffset = mz;

            // For each orientation, fill the bitmap of occupied cubes relative to the handle piece.
            FitWord = 0;
            for (c=0;c<32;c++){
                int x,y,z;
                x = FitMap[c].x;
                y = FitMap[c].y+my;
                z = FitMap[c].z+mz;
                if (y < 0 || y >= 5) continue; // Out of bounds
                if (z < 0 || z >= 5) continue; // Out of bounds
                if (Map.Data[x][y][z]){
                    FitWord |= (1<<c);
                }
            }
            AllPieces[a].FitOpt[b].FitWord = FitWord;
        }
    }
}

// generate empty field.
// function to test placement.
// recursive loop to place the pieces.
// Make 32 bit fit test opimisation bitmap.



typedef struct {
    char IsUsed[NUM_PIECES];
    char Map[FIELD_X_SIZE+1][FIELD_Y_SIZE+1][FIELD_Z_SIZE+1]; 
       // y and z require guard just one side (using array wrap around property)
       // x requres no lower bound.

}Field_t;


//-------------------------------------------------------------------------------
// Show the playing field.
//-------------------------------------------------------------------------------
void ShowMap(Field_t * Field, int HilightedCubenum)
{
    Map_3d_t ShowData;
    ShowData.Data = (char *)Field->Map;
    ShowData.z_incr = 1;
    ShowData.y_incr = FIELD_Z_SIZE+1;
    ShowData.x_incr = (FIELD_Z_SIZE+1)*(FIELD_Y_SIZE+1);

    ShowData.x_max = FIELD_X_SIZE;//+1;
    ShowData.y_max = FIELD_Y_SIZE;//+1;
    ShowData.z_max = FIELD_Z_SIZE;//+1;

    Show3dMap(ShowData, TRUE, HilightedCubenum);
}

//-------------------------------------------------------------------------------
// Show the playing field.
//-------------------------------------------------------------------------------
void ShowMap_Backside(Field_t * Field)
{
    Map_3d_t ShowData;
    ShowData.Data = ((char *)Field->Map) 
        + 1                                   * (FIELD_Z_SIZE-1)
        + (FIELD_Z_SIZE+1)                    *(FIELD_Y_SIZE-1);

    ShowData.z_incr = -1;
    ShowData.y_incr = -(FIELD_Z_SIZE+1);
    ShowData.x_incr = (FIELD_Z_SIZE+1)*(FIELD_Y_SIZE+1);

    ShowData.x_max = FIELD_X_SIZE;//+1;
    ShowData.y_max = FIELD_Y_SIZE;//+1;
    ShowData.z_max = FIELD_Z_SIZE;//+1;

    Show3dMap(ShowData, TRUE, -1);
}

//-------------------------------------------------------------------------------
// Attempt to place a piece.  returns TRUE if successful.
//-------------------------------------------------------------------------------
int CheckPlacement(Field_t * Field, int xp, int yp, int zp, int PieceNum, int Orientation)
{
    Map_t * ThePiece;
    int x,y,z;

    ThePiece = &AllPieces[PieceNum].Orientations[Orientation];
    yp -= AllPieces[PieceNum].FitOpt[Orientation].YOffset;
    zp -= AllPieces[PieceNum].FitOpt[Orientation].ZOffset;
    if ( yp < 0 || zp < 0){
        //printf("went negative\n");
        return FALSE; // filling desired cube goes out of bounds.
    }

    for (x=0;x<5;x++){
        for (y=0;y<5;y++){
            for (z=0;z<5;z++){
                if (Field->Map[x+xp][y+yp][z+zp] && ThePiece->Data[x][y][z]){
                    // Interferes.
                    //printf("Interferes at %d,%d,%d\n",x,y,z);
                    return FALSE;
                }
            }
        }
    }

    // Placement is possible.
    if (Field->IsUsed[PieceNum]){
        printf("Error! Doubly placed piece\n");
        exit(-1);
    }
    return TRUE;
}

//-------------------------------------------------------------------------------
// Attempt to place a piece.  returns TRUE if successful.
//-------------------------------------------------------------------------------
void PlacePiece(Field_t * Field, int xp, int yp, int zp, int PieceNum, int Orientation)
{
    Map_t * ThePiece;
    int x,y,z;

    ThePiece = &AllPieces[PieceNum].Orientations[Orientation];
    yp -= AllPieces[PieceNum].FitOpt[Orientation].YOffset;
    zp -= AllPieces[PieceNum].FitOpt[Orientation].ZOffset;
    if ( yp < 0 || zp < 0){
        printf("Error! went negative\n");
        exit(-1);
    }

    // Placement is possible.
    if (Field->IsUsed[PieceNum]){
        printf("Error! Doubly placed piece\n");
        exit(-1);
    }
    Field->IsUsed[PieceNum] = 1;

    for (x=0;x<5;x++){
        for (y=0;y<5;y++){
            for (z=0;z<5;z++){
                if (ThePiece->Data[x][y][z]){
                    if (Field->Map[x+xp][y+yp][z+zp]){
                        printf("Placing piece with interference!");
                        exit(-1);
                    }

                    Field->Map[x+xp][y+yp][z+zp] = PieceNum+1;
                }
            }
        }
    }
    //if (NumPlaced > 31) printf("\nNumPlaced borked\n");
    PlacementsTried += 1;
}



//-------------------------------------------------------------------------------
// Compute the fit word for fitting pre-checks.
//-------------------------------------------------------------------------------
unsigned long ComputeFitWord(Field_t * Field, int px,int py,int pz)
{
    int x,y,z,c;
    unsigned long FitWord = 0;
    for (c=0;c<32;c++){
        x = FitMap[c].x+px;
        y = FitMap[c].y+py;
        z = FitMap[c].z+pz;
        //if (x >= FIELD_X_SIZE) goto occupied;
        //if (y < 0 || y >= FIELD_Y_SIZE) goto occupied;
        //if (z < 0 || z >= FIELD_Z_SIZE) goto occupied;
        if (Field->Map[x][y][z]){
            FitWord |= (1<<c);
        }
    }
    return FitWord;
}

typedef struct {
    int PieceNum;
    int x,y,z;
    int Orientation;
}PlacedPos_t;

PlacedPos_t Placed[NUM_PIECES+100];
int NumPlaced;

//-------------------------------------------------------------------------------
// Show the puzzle solution.
//-------------------------------------------------------------------------------
void ShowSolution(Field_t * Field)
{
    int Distance[NUM_PIECES];
    
    #ifdef TEST_MODULE
    return;
    #endif
    
    printf("\nA solution:\nFront:\n");
  
    ShowMap(Field, -1);
 
    printf("Back:\n");
    ShowMap_Backside(Field);


    // Determine order of pieces for best visibility in solution display
    {
        int x,y,z,a;
        for (a=0;a<NUM_PIECES;a++){
            Distance[a] = 10000;
        }

        for (x=0;x<FIELD_X_SIZE;x++){
            for (y=0;y<FIELD_Y_SIZE;y++){
                for (z=0;z<FIELD_Z_SIZE;z++){
                    int ViewerDistance, PieceNum;
                    ViewerDistance = 200-x*1-y*3+z*1;
                    PieceNum = Field->Map[x][y][z]-1;
                    if (Distance[PieceNum] > ViewerDistance){
                        Distance[PieceNum] = ViewerDistance;
                    }
                }
            }
        }

        for (a=0;a<NumPlaced;a++){                                        
            //printf("piece %d distance %d\n",Placed[a].PieceNum, Distance[Placed[a].PieceNum]);
        }
    }

    // Sort the pieces of the solution by distance to viewer, starting with furthest
    {
        int a,b,Furthest, FurthestIndex;
        for (a=0;a<NumPlaced;a++){
            PlacedPos_t Temp;
            Furthest = -10000;
            for (b=a;b<NumPlaced;b++){
                int Dist;
                Dist = Distance[(int)Placed[b].PieceNum];
                if (Dist > Furthest){
                    Furthest = Dist;
                    FurthestIndex = b;
                }
            }
            // Swap.
            Temp = Placed[a];
            Placed[a] = Placed[FurthestIndex];
            Placed[FurthestIndex] = Temp;
        }
    }

    {
        Field_t Field;
        int a;
        memset(&Field, 0, sizeof(Field));
        for (a=0;a<NumPlaced;a++){
            PlacedPos_t p;
            p = Placed[a];
            printf("\n\n");
            printf("Piece %d\n",p.PieceNum);
            Show5(&AllPieces[p.PieceNum].Orientations[p.Orientation]);
            PlacePiece(&Field, p.x,p.y,p.z,p.PieceNum,p.Orientation);
            ShowMap(&Field, p.PieceNum+1);
        }
    }
}

Field_t Stages[NUM_PIECES+1];
int BackupTo;

//-------------------------------------------------------------------------------
// Recursive puzzle solving...
//-------------------------------------------------------------------------------
void SolvePuzzle(int px,int py,int pz)
{
    Field_t * Field;
    Field = &Stages[NumPlaced];
    
    
    if (NumPlaced == NUM_PIECES){
        // All placed is solved even if we have space left over.
        ShowSolution(Field);
        BackupTo = -1; // Don't look for more solution, just back out of recursion
        return;
    }
   

    Stages[NumPlaced+1] = Stages[NumPlaced];

    // Find next empty cube.
    while (Field->Map[px][py][pz]){
        if (py & 1){
            // Odd rows go backwards to improve locality & immediacy of undo.
            if (pz <= 0) goto next_y;
            pz -= 1;
        }else{
            if (pz >= (FIELD_Z_SIZE-1)){
                next_y:
                py += 1;
                if (py > FIELD_Y_SIZE){
                    py = 0;
                    pz = 0; // Must reset z for odd sizes of Y.
                    px++;
                    if (px >= FIELD_X_SIZE){
                        // All positions filled is solved
                        // even if we have pieces left over.
                        ShowSolution(Field);
                        BackupTo = -1; // Don't look for more solutions, just back out
                        return;
                    }
                }
            }else{
                pz += 1;
            }
        }
    }

    {
        unsigned long FitWord;
        int NumFits;
        int TryLevel;
        TryLevel = NumPlaced;
back_up_one:
        Field = &Stages[TryLevel];
        FitWord = ComputeFitWord(Field, px, py, pz);
        NumFits = 0;
        // Now find a piece to fit.
        {
            int PieceNum, or;
            for (PieceNum=0;PieceNum<NUM_PIECES;PieceNum++){
                if (Field->IsUsed[PieceNum]) continue; // Piece already used up.
                for (or=0;or<AllPieces[PieceNum].NumOrientations;or++){
                    if (FitWord & AllPieces[PieceNum].FitOpt[or].FitWord) continue;
                    if (CheckPlacement(Field, px,py,pz, PieceNum, or)){
                        NumFits += 1;
                        if (TryLevel != NumPlaced){
                            // We are testing if backing up by a move makes filling a certain
                            // cube possible.
                            // As we now know is that it is possible, no need to go further.
                            goto backout_shortcut;
                        }
                        Stages[NumPlaced+1] = Stages[NumPlaced];
                        PlacePiece(&Stages[NumPlaced+1], px,py,pz, PieceNum, or);
                        if (NumPlaced > 31) printf("\nNumPlaced borked 2\n");
                    
                        Placed[NumPlaced].PieceNum = PieceNum;
                        Placed[NumPlaced].Orientation = or;
                        Placed[NumPlaced].x = px;
                        Placed[NumPlaced].y = py;
                        Placed[NumPlaced].z = pz;
                        NumPlaced += 1;
                        NumFits += 1;

                        #ifndef TEST_MODULE
                        {
                            // Show current state of work to show progress if it takes a long time.
                            static int div;
                            if (div++ > 200000){
                                printf("x=%d\n",px);
                                ShowMap(Field, -1);
                                div = 0;
                            }
                        }
                        #endif

                        SolvePuzzle(px,py,pz);
                        // Returns if nothing worked or we finished this solution.
                        // Now un-place the piece for the next try.

                        NumPlaced -= 1;

                        if (BackupTo < NumPlaced){
                            // In attempting to fill a cube in some level of recursion down
                            // from here, we found that it could only be filled by unplacing a number
                            // of pieces, so we just pop the levels of recursion.
                            // printf("abort at level %d\n",NumPlaced);
                            return;
                        }else{
                            BackupTo = 1000;
                        }
                    }
                }
            }
        }
backout_shortcut:
        if (NumFits == 0){
            if (px < FIELD_X_SIZE-1){
                // If no piece can be used to fill the poosition at px,py,pz, then back up
                // in the placed pieces until that square can be filled.  Rather than building
                // up again at every level, we first check how far we need to back up until 
                // our present target cube is fillable.  However, we don't do this on the last
                // level, because the way we check placements, only flat pieces could be
                // used in the last level.
                // This optimisation speeds up the program by more than an order of magnitude.
                // printf("could not fill %d %d %d\n",px,py,pz);
                if (TryLevel == 0){
                    printf("Internal error\b");
                    exit(-1);
                }
        
                TryLevel -= 1;
                goto back_up_one;
            }
        }else{
            if (TryLevel != NumPlaced){
                // If we find that a lot of stuff needs to get unplaced in order to fill the
                // target cube, then there's no point in exploring all the remaining possibilites
                // of filling the squares betwen the level we had to back up to and the present
                // target cube.  Setting of "BackupTo" causes levels of recursion to subsequently
                // just pop off.
                
                //printf("Should back up from %2d to %2d\n",NumPlaced, TryLevel);
                BackupTo = TryLevel;
                // And return.
            }
        }
    }
}

//-------------------------------------------------------------------------------
// Show picutres of the pieces
//-------------------------------------------------------------------------------
void ShowPieces(void)
{
    int a;
    for (a=0;a<NUM_PIECES;a++){
        Map_t Map;
        Map = AllPieces[a].Orientations[0];
        printf("Piece %d:\n",a+1);
        Show5_FrontAndBack(&Map);
        printf("\n");
    }
}

//-------------------------------------------------------------------------------
// Create an empty solution field with no pieces in it yet.
//-------------------------------------------------------------------------------
void InitEmtpyField(void)
{
    Field_t Field;
    // Initialize the empty field.
    {
        int x,y,z;
        memset(&Field, 0, sizeof(Field_t));
        for (y=0;y<FIELD_Y_SIZE+1;y++){
            for (z=0;z<FIELD_Z_SIZE+1;z++){
                Field.Map[FIELD_X_SIZE][y][z] = -1; // End boundary.
            }
        }
        for (x=0;x<FIELD_X_SIZE;x++){
            for (y=0;y<FIELD_Y_SIZE+1;y++){
                Field.Map[x][y][FIELD_Z_SIZE] = -1; // Top boundary
            }
            for (z=0;z<FIELD_Z_SIZE+1;z++){
                Field.Map[x][FIELD_Y_SIZE][z] = -1; // Side boundary.
            }
        }
    }

    //ShowMap(&Field);
    PlacementsTried = 0;
    NumPlaced = 0;
    BackupTo = 1000;
    Stages[0] = Field;
}


#ifdef TEST_MODULE
//-------------------------------------------------------------------------------
// Just find a solution for benchmarking
//-------------------------------------------------------------------------------
int Time3dPentominoSolver(void)
{
    PreparePieces();
    InitEmtpyField();
    SolvePuzzle(0,0,0);
    return PlacementsTried;
}    
#else

//-------------------------------------------------------------------------------
// Solve it and show a solution.
//-------------------------------------------------------------------------------
int main(int argc, char * argv[])
{
    int ShowPiecesFlag = 0;
    int a;

    for (a=1;a<argc;a++){
        if (!strcmp(argv[a], "pieces")){
            ShowPiecesFlag = TRUE;
        }else{
            printf("Option %s not understood\n",argv[a]);
            exit(-1);
        }
    }

    PreparePieces();

    if (ShowPiecesFlag){
        ShowPieces();
        exit(0);
    }

    InitEmtpyField();
    
    SolvePuzzle(0,0,0);
    return 0;
}
#endif

