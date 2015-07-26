#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

class CLogger
{
public:
	CLogger()
	{
		m_pFile = fopen("test.log", "a");
	}

	~CLogger()
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
		va_list args;
	    va_start(args, fmt);
	    int n = vsnprintf(szBuf, 255, fmt, args);
	    va_end(args);
	    szBuf[255] = 0;
	    //printf("%s\n", szBuf);
	    puts(szBuf);
	    if(n < 0 )
	    {
	    	cout<<"error"<<endl;
	    	return;
	    }
	    if(m_pFile)
	    {
	    	fprintf(m_pFile, "%s\n", szBuf);
	    	fflush(m_pFile);
	    	fsync(fileno(m_pFile));
	    }
	}
private:
	FILE* m_pFile;
	
};

CLogger* g_pLogger = new CLogger;

#define LogDebug(fmt, args...) \
	do \
	{ \
		g_pLogger->WriteLog("[%s, %u]:" fmt, __FILE__, __LINE__, ##args); \
	}while(0)
void test_step()
{
    int i = 0;
    int sum = 0;
    for(i = 0; i < 10; ++i)
    {
        sum += i;
        printf("%d\n", sum);
    }
}
int main()
{
	int i = 0;
    test_step();
	while(i < 10)
	{
		LogDebug("hello");
		LogDebug("hello %d world", i);
		char szBuf[256];
		memset(szBuf, 0, 256);
		snprintf(szBuf, 256, "hello %d world", i);
		printf("buf:%s\n", szBuf);
		++i;
	}
	delete g_pLogger;
	g_pLogger = NULL;
	return 0;
}
