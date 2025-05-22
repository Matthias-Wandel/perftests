//----------------------------------------------------------------------------
// Compare crc timing calculations with predictable and unpredictable branches
// compile without optimizations, as optimizations appear to get rid of the
//    branches in the if_else crc routine.
//----------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
    #define _WINDOWS 1
#endif
#ifdef _WIN64
    #define _WINDOWS 1
#endif

#ifndef _WINDOWS
    #define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#ifdef _WINDOWS
    #include <windows.h>
    #if _MSC_VER < 1300
        // Make it work with the compiler from 2008 on my windows XP box.
        #define GetCurrentProcessorNumber() 0
    #endif
#else
    #include <sched.h>
    #include <unistd.h>
    #include <stdint.h>
    #include <sys/utsname.h>

    #define Sleep(a) usleep((a)*1000)
    #ifdef __linux__
        #define GetCurrentProcessorNumber() sched_getcpu()
    #else
        // Assume OS-X.  Doesn't appear to be a good way to get core number.
        #define GetCurrentProcessorNumber() 0
    #endif

#endif
#include "perftest.h"

int PreSleep = 0;
int CoresRunOn[30][2];

//----------------------------------------------------------------------------
// Get computer name for logging purposes.
//----------------------------------------------------------------------------
char * PcName(void)
{
#ifdef _WINDOWS
    static char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    if (!GetComputerName(computerName, &size)) {
        printf("Failed to get computer name (Error: %lu)\n", GetLastError());
    }

    // Some of my PCs I didn't rename to start wityh year of processor release
    if (strcmp("DESKTOP-4SS178E",computerName) == 0) return "2010-i5 refurb";
    if (strcmp("HP-SLIMLINE",computerName) == 0) return "2008-pentium";
    if (strcmp("I7NEW",computerName) == 0) return "2019-i7";

    return computerName;
#else
    FILE *fp = fopen("/proc/device-tree/model", "r");
    if (!fp) {
        perror("Failed to open /proc/device-tree/model");
        return "linux?";
    }

    char model[256];
    size_t len = fread(model, 1, sizeof(model) - 1, fp);
    fclose(fp);

    if (len > 0) {
        model[len] = '\0';  // Null-terminate the string
        static char ModelString[300];
        sprintf(ModelString,"%s", model);
        return ModelString;
    } else {
        return ("linux??");
    }
#endif
}

#ifndef _WINDOWS
//----------------------------------------------------------------------------
// Dump more info about the system to the file
// if its not me running the test, hard to get that info after the fact
//----------------------------------------------------------------------------
void DumpLinuxSystemInfo(char * filename)
{
    struct utsname userinfo;
    FILE * outfile;

    outfile = fopen(filename,"a");
    if(outfile){
        fprintf(outfile,"------------------------------------------------\n");
        if(uname(&userinfo)>=0){
            fprintf(outfile,"System Name:%s    Node:%s\n",userinfo.sysname,userinfo.nodename);
            fprintf(outfile,"System Release:%s   Version: %s\n",userinfo.release,userinfo.version);
            fprintf(outfile,"Machine: %s\n",userinfo.machine);
        }else{
            fprintf(outfile,"System details fetch failed..\n");
        }
        fclose(outfile);
    }
    if (system("lscpu >> results.csv")){
        printf("Error: lscpu failed\n");
    }
}
#endif

//----------------------------------------------------------------------------
// Put the time into the output file
//----------------------------------------------------------------------------
void PrintTimeToFile(FILE * outfile)
{
    time_t now;
    struct tm * local;
    char buffer[100];

    now = time(NULL);  // get current time
    local = localtime(&now);  // convert to local time
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local);
    fprintf(outfile, "Test run at: %s\n", buffer);
}



//----------------------------------------------------------------------------
// Make random-ish test data to test the CRC on.
//----------------------------------------------------------------------------
unsigned char * MakeDataToCrc(int size)
{
    // Allocate buffer
    int a;
    unsigned char * buffer = (uint8_t *)malloc(size);

    for (a=0;a<size;a++){
        buffer[a] = (unsigned char)(a - a/71 + a*53 + 0xf0);
    }
    return buffer;
}

#define NUM_CRC_MULTI 12
#define NUM_TESTS 6 // Not counting crc multi benchmarks
const char * Methods[] = {"CRC Table  ", "CRC and_xor", "CRC if_else", "CRC if-cnt ",
                          "Pentomino  ", "3dPentomino"};

//----------------------------------------------------------------------------
// Time various routines
//----------------------------------------------------------------------------
double TimeFunction(int WhichOne, uint8_t * buffer, int size)
{
    static int crc0=0;
    double duration_sec;
    const int NumIter = 1000;
    int iter;
    int core_start,core_after;
    int Malfunctioned = 0;

    #ifdef _WINDOWS
        LARGE_INTEGER freq_t, start_t, end_t;
    #endif


    if (PreSleep){
        // Pre-sleeping bumps program to a slower "efficiency core"
        // with core i9 and windows 11
        printf("(sleep %ds)  ",PreSleep);
        Sleep(PreSleep*1000);
    }

    if (WhichOne < NUM_TESTS){
        printf("%s",Methods[WhichOne]);
    }else{
        printf("CRC-MULTI %2d",WhichOne-10);
    }
    core_start = GetCurrentProcessorNumber();

    #ifdef _WINDOWS
        QueryPerformanceFrequency(&freq_t);
        QueryPerformanceCounter(&start_t);
    #else
        struct timespec ts_start, ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
    #endif

    if (WhichOne < 4){
        // CRC benchmarks
        int zeros;
        int crc = 0;
        for (iter=0;iter<NumIter;iter++){
            uint8_t * addr = buffer + iter*8;
            int size_use = size-NumIter*8;
            switch(WhichOne){
                case 0:
                    crc = compute_crc32_table(addr, size_use, &zeros);
                    if (iter == 0) crc0 = crc;
                    break;
                case 1:
                    crc = compute_crc32_and_xor(addr, size_use, &zeros);
                    break;
                case 2:
                    crc = compute_crc32_if_else(addr, size_use);
                    break;
                case 3:
                    crc = compute_crc32_if_else_count(addr, size_use, &zeros);
                    break;
            }
            if (iter ==0 && crc != crc0){
                printf("Error! CRCs mismatch %x %x\n",crc0, crc);
            }
        }

    }else if (WhichOne < 10){
        // Pentomino benchmark
        if (WhichOne == 4){
            int ret;
            ret = PentominoBenchmark();
            if (ret != 2339){
                // 2D Pentomino program malfunctions on Pi with /Ofast
                printf("Pentomino test malfunctioned\n");
                Malfunctioned = 1;
            }
        }else if (WhichOne == 5)
            Time3dPentominoSolver();
        else
            printf("None");

    }else if (WhichOne <= 30){
        // Simultaneous CRC benchmark
        for (iter=0;iter<NumIter;iter++){
            uint8_t * addr = buffer + iter*8;
            int size_use = size-NumIter*8;

            const int num = WhichOne-10;
            uint32_t crc_ret[20];

            unsigned char *buf[20];
            int a;
            for (a=0;a<num;a++) buf[a] = addr+a*8;

            compute_crc32_simul_n(buf, size_use, num, crc_ret);
        }
    }

    #ifdef _WINDOWS
        QueryPerformanceCounter(&end_t);
        duration_sec = (double)(end_t.QuadPart - start_t.QuadPart) / freq_t.QuadPart;
    #else
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        duration_sec = ts_end.tv_sec  - ts_start.tv_sec
            + (double)(ts_end.tv_nsec - ts_start.tv_nsec)/ 1e9;
    #endif
    if (Malfunctioned) duration_sec = -1;

    core_after = GetCurrentProcessorNumber();

    printf(", Core %2d-%2d",core_start,core_after);
    printf(", Time: %6.3f s\n",duration_sec);

    CoresRunOn[WhichOne][0] = core_start;
    CoresRunOn[WhichOne][1] = core_after;

    return duration_sec;
}


#ifdef _WINDOWS
void SetProcessorAffinity(int n) {
    DWORD_PTR mask = 1ULL << n; 

    HANDLE thread = GetCurrentThread();
    DWORD_PTR result = SetThreadAffinityMask(thread, mask);

    if (result == 0) {
        // Failed
        DWORD error = GetLastError();
        printf("Failed to set affinity. Error code: %lu\n", error);
    } else {
        // Success
        printf("Affinity successfully set to processor %d\n",n);
    }
}
#endif


//----------------------------------------------------------------------------
// Main
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    unsigned char *buffer;
    double Times[30];
    int a;
    int BufferSize = 100000;
    int TestStartAt = 0;
    int TestEndAt = 10+NUM_CRC_MULTI;
    int Repetitions = 1;
    int n;
    char AboutString[100];

    buffer = MakeDataToCrc(BufferSize+100);
    init_crc32_table();

    // Parse command line options
    for (a=1;a<argc;a++){
        if (argv[a][0] == '-'){
            int num = 0;
            num = atoi(argv[a]+2);

            if (argv[a][1] == 't'){
                // For running a subset of tests, as throttling may happen
                // before the tests are done.  Also for running individual tests
                TestStartAt = num;
                TestEndAt = num;
                char * dash = strchr(argv[a]+2, '-');
                if (dash){
                    int e = atoi(dash+1);
                    if (e) TestEndAt = e;
                    TestEndAt = 20;
                }else{
                    TestEndAt = num;
                }
                printf("Run tests %d to %d\n",TestStartAt, TestEndAt);
            }
            if (argv[a][1] == 'r'){
                Repetitions = num;
                printf("Repeat %d times\n",Repetitions);
            }
            if (argv[a][1] == 's'){
                PreSleep = num;
                printf("Pre-sleep %d seconds\n",PreSleep);
            }
            if (argv[a][1] == 'a'){
                #ifdef _WINDOWS
                    SetProcessorAffinity(num);
                #else
                    printf("Affinity setting unavailble in this build\n",PreSleep);
                #endif
            }
        }
    }


    printf("Matthias's little performance benchmarks\n");

    // String identifying which compilation and which computer running on.
    #ifdef _MSC_VER
        printf("Compiled: %dbit,  MSVC %5.2f, Optimization='%s',%s\n",(int)sizeof(int *)*8, _MSC_VER/100.0,OPTFLAG, PcName());
        sprintf(AboutString,"MSVC%d %db %-6.6s,%-14.14s", _MSC_VER/100,
              (int)sizeof(int *)*8,OPTFLAG, PcName());
    #else
        printf("Compiled: %dbit,  GCC %d, Optimization='%s',%s\n",(int)sizeof(int *)*8, __GNUC__,OPTFLAG, PcName());
        sprintf(AboutString,"GCC%d %db %-6.6s,%-14.14s", __GNUC__,
              (int)sizeof(int *)*8,OPTFLAG, PcName());
    #endif


    FILE * outfile = stdout;
    // Time the different tests
    memset(Times, 0, sizeof(Times));
    printf("Run tests:\n");
    for (a=TestStartAt;a<=TestEndAt;a++){
        if (a<NUM_TESTS || a > 10){
            for (int r=0; r<Repetitions;r++){
                Times[a] = TimeFunction(a, buffer,BufferSize);
            }
        }
    }

    // Print results to console
    if (n == 0){ // Print legend
        printf("Compiled         ,Computer      ");
        for (a=0;a<NUM_TESTS;a++) printf(",%s",Methods[a]);
        printf("\n");
    }

    printf("%s",AboutString);
    // Print the timing results.
    for (a=0;a<NUM_TESTS;a++) printf(", %10.3f",Times[a]);
    printf("\n");

    printf("%s,CRCMulti",AboutString);
    // Print the timing results.
    for (a=0;a<12;a++) printf(",%6.3f",Times[a+10]);
    printf("\n");


    // Print results to file.
    #ifndef _WINDOWS
        DumpLinuxSystemInfo("results.csv");
    #endif
    outfile = fopen("results.csv","a");
    if (!outfile){
        // If file is busy, try again in half a second.
        // cause I like to run this on multiple computers at the same time.
        Sleep(500);
        printf("Retry out file open\n");
        outfile = fopen("results.csv","a");
    }
    if (outfile){
        PrintTimeToFile(outfile);
        if (n == 0){ // Print legend
            fprintf(outfile,"Compiled         ,Computer      ");
            for (a=0;a<NUM_TESTS;a++) fprintf(outfile,",%s",Methods[a]);
            fprintf(outfile,"\n");
        }

        fprintf(outfile, "%s", AboutString);
        // Print the timing results.
        for (a=0;a<NUM_TESTS;a++) fprintf(outfile,", %10.3f",Times[a]);
        fprintf(outfile,"\n");

        fprintf(outfile, "%s,CRCMulti",AboutString);
        // Print the timing results.
        for (a=0;a<12;a++) fprintf(outfile,",%6.3f",Times[a+10]);
        fprintf(outfile,"\n");

        fprintf(outfile,"Cores run on:");
        for (a=1;a<10+NUM_CRC_MULTI;a++){
            if (a < NUM_TESTS || a >= 10){
                fprintf(outfile," (%d,%d)",CoresRunOn[a][0],CoresRunOn[a][1]);
            }else{
                fprintf(outfile," ");
            }
        }
        fprintf(outfile,"\n");

        fclose(outfile);
    }else{
        printf("ERROR!  ERROR!  Unable to open results file for writing!\n");
    }

    free(buffer);
    return 0;
}
