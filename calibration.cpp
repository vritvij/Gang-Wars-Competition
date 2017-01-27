//  Windows
#ifdef _WIN32

#include <Windows.h>

double get_wall_time() {
    LARGE_INTEGER time, freq;
    if (!QueryPerformanceFrequency(&freq)) {
        //  Handle error
        return 0;
    }
    if (!QueryPerformanceCounter(&time)) {
        //  Handle error
        return 0;
    }
    return (double) time.QuadPart / freq.QuadPart;
}

double get_cpu_time() {
    FILETIME a, b, c, d;
    if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0) {
        //  Returns total user time.
        //  Can be tweaked to include kernel times as well.
        return (double) (d.dwLowDateTime | ((unsigned long long) d.dwHighDateTime << 32)) * 0.0000001;
    } else {
        //  Handle error
        return 0;
    }
}

//  Posix/Linux
#else
#include <time.h>
#include <sys/time.h>
double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}
#endif

#include <fstream>
#define NodeCount 1000000000

using namespace std;

struct moveDS {
	int Row, Column, Type; //Stake = 0, Raid = 1
};

struct node {
    char **BoardState;
    moveDS Move, BestNextMove;
	int Score, AvailableMoves;
    char Turn;              //X or O and is identifies who will play the next ply
};

int main() {
    double WallTime = get_wall_time();
    double CpuTime = get_cpu_time();

    for (long long i = 0; i < NodeCount; i++) {
        node x;
    };

    WallTime = get_wall_time() - WallTime;
    CpuTime = get_cpu_time() - CpuTime;

    ofstream OutputFile("CPU.txt");
    if (OutputFile.is_open()) {
        //Write time required per node
        OutputFile << (WallTime / NodeCount) << endl;
        OutputFile << (CpuTime / NodeCount) << endl;
        OutputFile.close();
    }

    //cout << "Wall Time = " << WallTime << endl;
    //cout << "CPU Time  = " << CpuTime << endl;

    return 0;
}