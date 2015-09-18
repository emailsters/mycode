/*
    @compile: g++ -o full_memory full_memory.cpp -g
    @usage: ./fill_memory 200
    @author: kewenchen
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>
#include <vector>
#include <sys/stat.h>
#include <stdarg.h>
#include <signal.h>

using namespace std;

#define LOG_FILE_NAME "fill_memory.log"

static bool g_running = false;

class Logger{
public:
    Logger()
    {
        log_file_ = fopen(LOG_FILE_NAME, "a");
    }
    virtual ~Logger()
    {
        if (log_file_)
        {
            fclose(log_file_);
        }
    }

    void WriteLog(const char* fmt, ...)
    {
        char buf[256];
        (void)memset(buf, 0, 256);
        int nLen = SetTime(buf, 256);
        va_list arg;
        va_start(arg, fmt);
        vsprintf(buf + nLen, fmt, arg);
        va_end(arg);
        buf[255] = 0;
        if (log_file_)
        {
            fprintf(log_file_, "%s\n", buf);
            fflush(log_file_);
            fsync(fileno(log_file_));
        }       
    }

    int SetTime(char* buf, int nLen)
    {
        time_t rawtime;
        struct tm* timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        int len = snprintf(buf, nLen, "[%4d-%02d-%02d %02d:%02d:%02d]: ", 1900+timeinfo->tm_year, 
            1+timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,
            timeinfo->tm_sec);
        return len;
    }

private:
    FILE* log_file_;
};

Logger* g_logger = NULL;

#define Log(fmt, args...) \
    do \
    { \
        if (g_logger) \
        { \
            g_logger->WriteLog(fmt, ##args); \
        } \
    }while(0)


typedef struct tagMemInfo
{
    size_t mem_total;
    size_t mem_left;
    size_t swap_total;
    size_t swap_left;
    tagMemInfo() : mem_total(0), mem_left(0), swap_total(0), swap_left(0){}
} MemInfo;

#define DO_QUIT_LOOP(counter) \
    do \
    { \
        if (++counter == 4) \
        { \
            break; \
        } \
    }while(0)

void GetMemoryInfo(FILE*fp, MemInfo& meminfo)
{
    if (!fp)
    {
        cout<<"open file error"<<endl;
        return;
    }

    fseek(fp, 0, SEEK_SET);

    char key_name[256];
    int value;
    int uiIsDone = 0;
    while (!feof(fp))
    {
        fscanf(fp, "%s %d", key_name, &value);
        if (0 == strncmp("MemTotal:", key_name, 256))
        {
            meminfo.mem_total = value;
            DO_QUIT_LOOP(uiIsDone);
        }
        else if (0 == strncmp("MemFree:", key_name, 256))
        {
            meminfo.mem_left = value;
            DO_QUIT_LOOP(uiIsDone);
        }
        else if (0 == strncmp("SwapTotal:", key_name, 256))
        {
            meminfo.swap_total = value;
            DO_QUIT_LOOP(uiIsDone);
        }
        else if (0 == strncmp("SwapFree:", key_name, 256))
        {
            meminfo.swap_left = value;
            DO_QUIT_LOOP(uiIsDone);
        }
    }
}

vector<char*>* g_mem_blocks;

// 100M
#define BLOCK_SIZE (102400)
int g_mem_left = 0;

FILE* GetMemoryInfoHandle()
{
    FILE* fp = fopen("/proc/meminfo", "r");
    return fp;
}

void CloseMemoryInfoHandle(FILE* fp)
{
    fclose(fp);
}

void FillMemory(size_t size_to_fill)
{
    FILE* fp = GetMemoryInfoHandle();

    int target_size = (int)size_to_fill;
    Log("memory needed to fill:%uKB", target_size);

    for (; target_size > 0; )
    {
        size_t real_size = 0;

        (target_size >= BLOCK_SIZE) ? (real_size = BLOCK_SIZE) : (real_size = target_size); 

        char* p = (char*)malloc(real_size * 1024);
        if (!p)
        {
            Log("memory is full");
            break;
        }

        (void)memset(p, 0, real_size * 1024);
    
        MemInfo meminfo;
        GetMemoryInfo(fp, meminfo);
        Log("TargetSize:%dKB, AllocSize:%uKB, RemainSize:%uKB", target_size, real_size, 
            (meminfo.mem_left + meminfo.swap_left));

        g_mem_blocks->push_back(p);
        target_size = (int)meminfo.mem_left + (int)meminfo.swap_left - g_mem_left;
        usleep(500000);
    }

    CloseMemoryInfoHandle(fp);
    fp = NULL;
}

void FullMemory(void* arg)
{
    FILE* fp = GetMemoryInfoHandle();
    MemInfo meminfo;
    GetMemoryInfo(fp, meminfo);
    Log("MemTotal:%uKB, MemLeft:%uKB, SwapTotal:%uKB, SwapLeft:%uKB", 
        meminfo.mem_total, meminfo.mem_left, meminfo.swap_total, meminfo.swap_left);
    CloseMemoryInfoHandle(fp);

    FillMemory(meminfo.mem_left + meminfo.swap_left);
}

void CleanMemory()
{
    if(!g_mem_blocks) return;
    vector<char*>::iterator itor = g_mem_blocks->begin();
    for (; itor != g_mem_blocks->end(); ++itor)
    {
        if (*itor != NULL)
        {
            free(*itor);
        }
    }
    g_mem_blocks->clear();
}

void SignalHandle(int signo)
{
    Log("catch signal %d", signo);
    CleanMemory();
    delete g_logger;
    g_logger = NULL;
    delete g_mem_blocks;
    g_mem_blocks = NULL;
    Log("exit");
    exit(0);
}

typedef void (DaemonFunc)(void *);

void Daemon(DaemonFunc daemon_func)
{
    umask(0);

    pid_t pid = fork();
    if(pid < 0)
    {
        cout<<"fork error"<<endl;
    }
    else if (pid == 0)
    {
        struct sigaction sig;  
        sig.sa_handler = SignalHandle;  
        sigemptyset(&sig.sa_mask);  
        sig.sa_flags = 0;  
        sigaction(SIGINT, &sig, NULL);
        sigaction(SIGTERM, &sig, NULL);

        setsid();

        // would not change dir
        
        struct rlimit stlimit;
        if(getrlimit(RLIMIT_NOFILE, &stlimit) == 0)
        {
            for(int i = 0; i < stlimit.rlim_max; ++i)
            {
                close(i);
            }
        }

        g_logger = new Logger;
        if (!g_logger)
        {
            cout<<"g_logger is NULL, would not write log"<<endl;
        }

        g_mem_blocks = new vector<char*>;
        if (!g_mem_blocks)
        {
            Log("g_mem_blocks is NULL, exit");
            delete g_logger;
            g_logger = NULL;
            return;
        }

        Log("start to fill memory.");
        daemon_func(NULL);
        
        FILE* fp = GetMemoryInfoHandle();
        MemInfo meminfo;
        GetMemoryInfo(fp, meminfo);
        Log("fill memory ended. MemTotal:%dKB, MemLeft:%dKB, SwapTotal:%dKB, SwapLeft:%dKB", 
            meminfo.mem_total, meminfo.mem_left, meminfo.swap_total, meminfo.swap_left);
        CloseMemoryInfoHandle(fp);
        fp = NULL;
    }
    else
    {
        cout<<"start fill memory daemon. uses kill -15/-9 to kill the daemon when don't need it."<<endl;
        exit(0);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: ./fillmemory 200\n");
        return 0;
    }
    g_mem_left = atoi(argv[1]) * 1000;

    Daemon(FullMemory);
    while (1)
    {
        sleep(1000000);
    }

    return 0;
}

