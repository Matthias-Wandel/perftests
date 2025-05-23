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

typedef struct {
    int Affinity;
    double Times[30];
    int CoresRunOn[30][2];
}ThreadPassParms_t;


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
double TimeFunction(int WhichOne, int * CoresRunOn, uint8_t * buffer, int size)
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

    const char * str = NULL;
    if (WhichOne < NUM_TESTS){
        str = Methods[WhichOne];
    }else{
        char strbuf[20];
        sprintf(strbuf,"CRC_MULTI %2d",WhichOne-10);
        str = strbuf;
    }
    printf("%s, Core %2d-%2d, Time: %6.3f s\n",str,core_start,core_after,duration_sec);

    CoresRunOn[0] = core_start;
    CoresRunOn[1] = core_after;

    return duration_sec;
}


#ifdef _WINDOWS
void SetProcessorAffinity(int n)
{
    DWORD_PTR mask = 1ULL << n;

    HANDLE thread = GetCurrentThread();
    DWORD_PTR result = SetThreadAffinityMask(thread, mask);

    if (result == 0) {
        // Failed
        DWORD error = GetLastError();
        printf("Failed to set affinity. Error code: %lu\n", error);
    } else {
        // Success
        printf("Affinity set to processor %d\n",n);
    }
}

void SetProcessPriority(BOOL highPriority)
{
    HANDLE hProc = GetCurrentProcess();
    DWORD desiredClass = highPriority?HIGH_PRIORITY_CLASS:IDLE_PRIORITY_CLASS;

    if (!SetPriorityClass(hProc, desiredClass)) {
        DWORD err = GetLastError();
        fprintf(stderr, "Failed to set process priority (%lu)\n", err);
    } else {
        printf("Process priority set to %s_PRIORITY_CLASS\n",
               highPriority ? "HIGH" : "IDLE");
    }
}
#endif


static char AboutString[100];
static int BufferSize = 100000;
static unsigned char *buffer;

static int TestStartAt = 0;
static int TestEndAt = 10+NUM_CRC_MULTI;
static int Repetitions = 1;
static int Priority = -1;

//----------------------------------------------------------------------------
// Run the tests.
// There may be multiple instances of this running at the same time
//----------------------------------------------------------------------------
DWORD WINAPI DoTests(LPVOID param)
{
    ThreadPassParms_t * Parms = param;

    int Affinity = Parms->Affinity;
    if (Affinity > 0) SetProcessorAffinity(Affinity);
    if (Priority >= 0) SetProcessPriority(Priority);

    // Time the different tests
    for (int a=TestStartAt;a<=TestEndAt;a++){
        if (a<NUM_TESTS || a > 10){
            for (int r=0; r<Repetitions;r++){
                Parms->Times[a] += TimeFunction(a, Parms->CoresRunOn[a],buffer,BufferSize);
            }
        }
    }

    return 0;
}

#define MAX_PROCESSES 32
ThreadPassParms_t Parms[MAX_PROCESSES] = {0};
int ProcessorAffinities[MAX_PROCESSES] = {-1};
int NumAffinities = 0;


//----------------------------------------------------------------------------
// Print summary of overall results
//----------------------------------------------------------------------------
void PrintResults(FILE * outfile)
{
    int nres = NumAffinities? NumAffinities : 1;

    for (int n=0;n<nres;n++){
        double * Times = Parms[n].Times;
        int (*CoresRunOn)[2] = Parms[n].CoresRunOn;

        fprintf(outfile,"Compiled         ,Computer      ");
        for (int a=0;a<NUM_TESTS;a++) fprintf(outfile,",%s",Methods[a]);
        fprintf(outfile,"\n");

        if (TestStartAt < 10){
            fprintf(outfile,"%s",AboutString);
            // Print the timing results.
            for (int a=0;a<NUM_TESTS;a++) fprintf(outfile,", %10.3f",Times[a]/Repetitions);
            fprintf(outfile,"\n");

            fprintf(outfile,"Cores run on:");
            for (int a=1;a<NUM_TESTS;a++){
                if (CoresRunOn[a][0]==CoresRunOn[a][1]){
                    fprintf(outfile," %d,",CoresRunOn[a][0]);
                }else{
                    fprintf(outfile," %d->%d,",CoresRunOn[a][0],CoresRunOn[a][1]);
                }
            }
            fprintf(outfile,"\n");

        }

        if (TestEndAt > 10){
            fprintf(outfile,"%s,CRCMulti",AboutString);
            // Print the timing results.
            for (int a=0;a<12;a++) fprintf(outfile,",%6.3f",Times[a+10]/Repetitions);
            fprintf(outfile,"\n");
            fprintf(outfile,"Cores run on:");
            for (int a=10;a<22;a++){
                if (CoresRunOn[a][0]==CoresRunOn[a][1]){
                    fprintf(outfile," %d,",CoresRunOn[a][0]);
                }else{
                    fprintf(outfile," %d->%d,",CoresRunOn[a][0],CoresRunOn[a][1]);
                }
            }
            fprintf(outfile,"\n");
        }
        fprintf(outfile,"\n");
    }
}

//----------------------------------------------------------------------------
// Show command line options
//----------------------------------------------------------------------------
void Usage(void)
{
    printf("Matthias's little performance benchmark suite\n"
           "Uage: perftest <options>\n"
           "Options are:\n"
           "   -t[n]       Run only test [n]\n"
           "   -t[s]-[e]   Run only test [s] thru [e]\n"
           "   -r[n]       Repeat each thest [n] times\n"
           "   -p1         Set to run as high priority\n"
           "   -p0         Set to run as background priority\n"
           "   -a[n]       Set pricessor affinity to [n].  To run\n"
           "               multiple threads, specify -a[n] more than once.\n"
           "               if [n] is -1, this means any thread\n"
           );
    exit(-1);
}


//----------------------------------------------------------------------------
// Main
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Parse command line options
    for (int a=1;a<argc;a++){
        if (argv[a][0] == '-'){
            int num = 0;
            num = atoi(argv[a]+2);

            switch(argv[a][1]){
            case 't':
                // For running a subset of tests, as throttling may happen
                // before the tests are done.  Also for running individual tests
                TestStartAt = num;
                TestEndAt = num;
                char * dash = strchr(argv[a]+2, '-');
                if (dash){
                    TestEndAt = 20;
                    int e = atoi(dash+1);
                    if (e) TestEndAt = e;
                }else{
                    TestEndAt = num;
                }
                printf("Run tests %d to %d\n",TestStartAt, TestEndAt);
                break;

            case 'r':
                Repetitions = num;
                printf("Repeat %d times\n",Repetitions);
                break;

            case 'p':
                Priority = num;
                #ifndef _WINDOWS
                    printf("Priority setting unavailble in this build\n");
                #endif
                break;

            case 'a':
                if (NumAffinities < MAX_PROCESSES){
                    ProcessorAffinities[NumAffinities++] = num;
                    #ifndef _WINDOWS
                        printf("Affinity setting unavailble in this build\n");
                    #endif
                }else{
                    printf("Too many affinities specified");
                }
                break;
            default:
                printf("Argumant '%s' not understoond\n",argv[a]);
                Usage();
            }
        }else{
            printf("Argumant '%s' not understoond\n",argv[a]);
            Usage();
        }
    }


    printf("Matthias's little performance benchmarks\n");

    buffer = MakeDataToCrc(BufferSize+100);
    init_crc32_table();

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


    if (NumAffinities <= 1){
        Parms[0].Affinity = ProcessorAffinities[0];
        DoTests(&Parms[0]);
    }else{
#if 0 // Posix method, untested.
        Pthread_t threads[MAX_PROCESSES];

        for (int a=0;a<NumAffinities;a++){
            Parms[a].Affinity = ProcessorAffinities[a];
            if (pthread_create(&threads[a], NULL, DoTests, &Parms[a]) != 0) {
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }

        printf("Threads launched\n")
        // Wait for all threads to finish
        for (int a = 0; a < NumAffinities; a++) {
            pthread_join(threads[a], NULL);
            printf("thread %d done\n",a);
        }
#else
        HANDLE threads[MAX_PROCESSES];
        for (int a=0;a<NumAffinities;a++){
            Parms[a].Affinity = ProcessorAffinities[a];

            threads[a] = CreateThread(
                NULL,        // Default security attributes
                0,           // Default stack size
                DoTests,     // Thread function
                &Parms[a], // Argument to thread function
                0,           // Default creation flags
                NULL         // Thread ID not needed
            );
            Sleep(20);
        }
        printf("Threads launched\n");

        // Wait for all threads to finish
        WaitForMultipleObjects(NumAffinities, threads, TRUE, INFINITE);

#endif
    }
    free(buffer);

    PrintResults(stdout);

    // Then print the results to a file.
    FILE * outfile = fopen("results.csv","a");
    if (!outfile){
        Sleep(500); // If file is busy, try again in half a second.
                    // cause sometimes I run it on multiple computers on a network drive.
        printf("Retry output file open\n");
        outfile = fopen("results.csv","a");
    }
    if (outfile){
        PrintResults(outfile);
        fclose(outfile);
    }
}
