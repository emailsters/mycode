/*
    @compile: g++ -o full_memory full_memory.cpp -g
    @usage: ./full_memory
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

using namespace std;

#define LOG_FILE_NAME "full_memory.log"

static bool g_bRunning = false;

class CLogger{
public:
    CLogger()
    {
        m_pFile = fopen(LOG_FILE_NAME, "a");
    }
    virtual ~CLogger()
    {
        if(m_pFile)
        {
            fclose(m_pFile);
            m_pFile = NULL;
        }
    }

    void WriteLog(const char* fmt, ...)
    {
        char szBuf[256];
        (void)memset(szBuf, 0, 256);
        int nLen = SetTime(szBuf, 256);
        va_list arg;
        va_start(arg, fmt);
        vsprintf(szBuf + nLen, fmt, arg);
        va_end(arg);
        szBuf[255] = 0;
        if(m_pFile)
        {
            fprintf(m_pFile, "%s\n", szBuf);
            fflush(m_pFile);
            fsync(fileno(m_pFile));
        }       
    }

    int SetTime(char* szBuf, int nLen)
    {
        time_t rawtime;
        struct tm* timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        int nCpLen = snprintf(szBuf, nLen, "[%4d-%02d-%02d %02d:%02d:%02d]: ", 1900+timeinfo->tm_year, 
            1+timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,
            timeinfo->tm_sec);
        return nCpLen;
    }

private:
    FILE* m_pFile;
};

CLogger* g_pChildLogger = NULL;

#define Log(fmt, args...) \
    do \
    { \
        if(g_pChildLogger) \
        { \
            g_pChildLogger->WriteLog(fmt, ##args); \
        } \
    }while(0)


typedef struct tagMemInfo
{
    size_t uiMemTotal;
    size_t uiMemLeft;
    size_t uiSwapTotal;
    size_t uiSwapLeft;
    tagMemInfo() : uiMemTotal(0), uiMemLeft(0), uiSwapTotal(0), uiSwapLeft(0){}
} MemInfo;

#define DO_QUIT_LOOP(counter) \
    do \
    { \
        if(++counter == 4) \
        { \
            break; \
        } \
    }while(0)

void GetMemoryInfo(FILE*fp, MemInfo& stMemInfo)
{
    if(!fp)
    {
        cout<<"open file error"<<endl;
        return;
    }

    fseek(fp, 0, SEEK_SET);

    char szBufName[256];
    int value;
    int uiIsDone = 0;
    while(!feof(fp))
    {
        fscanf(fp, "%s %d", szBufName, &value);
        if(0 == strncmp("MemTotal:", szBufName, 256))
        {
            stMemInfo.uiMemTotal = value;
            DO_QUIT_LOOP(uiIsDone);
        }
        else if(0 == strncmp("MemFree:", szBufName, 256))
        {
            stMemInfo.uiMemLeft = value;
            DO_QUIT_LOOP(uiIsDone);
        }
        else if(0 == strncmp("SwapTotal:", szBufName, 256))
        {
            stMemInfo.uiSwapTotal = value;
            DO_QUIT_LOOP(uiIsDone);
        }
        else if(0 == strncmp("SwapFree:", szBufName, 256))
        {
            stMemInfo.uiSwapLeft = value;
            DO_QUIT_LOOP(uiIsDone);
        }
    }
}

vector<char*>* g_pMemPointerVector;

// 100M
#define BLOCK_SIZE (102400)
#define MEM_LEFT (50000)

FILE* GetMemoryInfoHandle()
{
    FILE* fp = fopen("/proc/meminfo", "r");
    return fp;
}

void CloseMemoryInfoHandle(FILE* fp)
{
    fclose(fp);
}

void FillMemory(size_t uiSize)
{
    FILE* fp = GetMemoryInfoHandle();

    int uiTargetSize = (int)uiSize;
    Log("memory needed to fill:%uKB", uiTargetSize);

    for(; uiTargetSize > 0; )
    {
        size_t uiRealSize = 0;

        (uiTargetSize >= BLOCK_SIZE) ? (uiRealSize = BLOCK_SIZE) : (uiRealSize = uiTargetSize); 

        char* p = (char*)malloc(uiRealSize * 1024);
        if(!p)
        {
            Log("memory is full");
            break;
        }

        (void)memset(p, 0, uiRealSize * 1024);
    
        MemInfo stMemInfo;
        GetMemoryInfo(fp, stMemInfo);
        Log("TargetSize:%dKB, AllocSize:%uKB, RemainSize:%uKB", uiTargetSize, uiRealSize, 
            (stMemInfo.uiMemLeft + stMemInfo.uiSwapLeft));

        g_pMemPointerVector->push_back(p);
        uiTargetSize = (int)stMemInfo.uiMemLeft + (int)stMemInfo.uiSwapLeft - MEM_LEFT;
        sleep(1);
    }

    CloseMemoryInfoHandle(fp);
    fp = NULL;
}

void FullMemory(void* pParam)
{
    FILE* fp = GetMemoryInfoHandle();
    MemInfo stMemInfo;
    GetMemoryInfo(fp, stMemInfo);
    Log("MemTotal:%uKB, MemLeft:%uKB, SwapTotal:%uKB, SwapLeft:%uKB", 
        stMemInfo.uiMemTotal, stMemInfo.uiMemLeft, stMemInfo.uiSwapTotal, stMemInfo.uiSwapLeft);
    CloseMemoryInfoHandle(fp);

    FillMemory(stMemInfo.uiMemLeft + stMemInfo.uiSwapLeft);
}

void CleanMemory()
{
    if(!g_pMemPointerVector) return;
    vector<char*>::iterator itor = g_pMemPointerVector->begin();
    while(itor != g_pMemPointerVector->end())
    {
        if(*itor != NULL)
        {
            free(*itor);
            (*itor) = NULL;
        }       
    }
    g_pMemPointerVector->clear();
}

typedef void (FuncDaemonAction)(void *);

void Daemon(FuncDaemonAction pfnDaemon)
{
    umask(0);

    pid_t pid = fork();
    if(pid < 0)
    {
        cout<<"fork error"<<endl;
    }
    else if(pid == 0)
    {
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

        g_pChildLogger = new CLogger;
        if(!g_pChildLogger)
        {
            cout<<"g_pChildLogger is NULL, would not write log"<<endl;
        }

        g_pMemPointerVector = new vector<char*>;
        if(!g_pMemPointerVector)
        {
            Log("g_pMemPointerVector is NULL, exit");
            delete g_pChildLogger;
            g_pChildLogger = NULL;
            return;
        }

        Log("start to fill memory.");
        pfnDaemon(NULL);
        
        FILE* fp = GetMemoryInfoHandle();
        MemInfo stMemInfo;
        GetMemoryInfo(fp, stMemInfo);
        Log("fill memory ended. MemTotal:%dKB, MemLeft:%dKB, SwapTotal:%dKB, SwapLeft:%dKB", 
            stMemInfo.uiMemTotal, stMemInfo.uiMemLeft, stMemInfo.uiSwapTotal, stMemInfo.uiSwapLeft);
        CloseMemoryInfoHandle(fp);
        fp = NULL;

        g_bRunning = true;
        while(g_bRunning)
        {
            sleep(10000000);
        }

        CleanMemory();
        
        delete g_pChildLogger;
        g_pChildLogger = NULL;
        delete g_pMemPointerVector;
        g_pMemPointerVector = NULL;
    }
    else
    {
        cout<<"start fill memory daemon. uses kill -9 to kill the daemon when don't need it."<<endl;
        exit(0);
    }
}

int main()
{
    Daemon(FullMemory);
    return 0;
}
