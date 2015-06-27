#include <stdio.h>
#include <iostream>
#include <string>
#include <list>
#include <unistd.h>
#include <string.h>

using namespace std;

enum ReturnStatus
{
    FAILED = -1,
    SUCCESS
};

typedef struct Record
{
    int nIndex;
    char szKey[127];
    char szValue[127];
    Record():nIndex(0)
    {
        szKey[0] = 0;
        szValue[0] = 0;
    }
}tagRecord;

class Coder 
{
public:
    virtual void Encode(tagRecord* pstSrc) = 0;
    virtual void Decode(tagRecord* pstSrc) = 0;
};

class SimpleCoder : public Coder
{
public:
    void Encode(tagRecord* pstSrc);
    void Decode(tagRecord* pstSrc);
private:
    void Des(char* pStr, int nLen);
};

inline void SimpleCoder::Des(char* pStr, int nLen)
{
    int i;
    for(i = 0; i < nLen; ++i)
    {
        pStr[i] ^= nLen - i;
    }
    pStr[i] = 0;
}

void SimpleCoder::Encode(tagRecord* pstSrc)
{
    if(!pstSrc)
    {
        return;
    }
    Des(pstSrc->szKey, 126);
    Des(pstSrc->szValue, 126);
}

void SimpleCoder::Decode(tagRecord* pstSrc)
{
    if(!pstSrc)
    {
        return;
    }
    Des(pstSrc->szKey, 126);
    Des(pstSrc->szValue, 126);
}

class PasswdOperator{
public:
    PasswdOperator(string strFilename);
    virtual ~PasswdOperator();
    
    int Init();
    void Write(tagRecord* pstRecord);
    void Find(const string& strKey, tagRecord* pstRecord);
    void FindAll();
    void PrintRecord(const tagRecord& stRecord);

private:
        void Read(tagRecord* pstRecord);

private:
    Coder* m_pCoder;
    string m_strFileName;
    FILE* m_pFile;
    list<tagRecord> m_lRecordList;
};

PasswdOperator::PasswdOperator(string filename) : m_strFileName(filename), m_pCoder(NULL), m_pFile(NULL)
{
    m_lRecordList.clear();
}

PasswdOperator::~PasswdOperator()
{
    if(m_pCoder)
    {
        delete m_pCoder;
        m_pCoder = NULL;
    }
    if(m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}

inline void PasswdOperator::PrintRecord(const tagRecord& record)
{
    printf("index:%d, key:%s, value:%s\n", record.nIndex, record.szKey, record.szValue);
}

int PasswdOperator::Init()
{
    if(m_strFileName.empty()) return FAILED;
    if(m_pFile) fclose(m_pFile);

    m_pFile = fopen(m_strFileName.c_str(), "a+");
    printf("open file %s success\n", m_strFileName.c_str());

    if(!m_pFile) return FAILED;

    m_pCoder = new SimpleCoder;    
    
    return SUCCESS;
}

void PasswdOperator::Write(tagRecord* pstRecord)
{
    if(m_pCoder)
    {
        m_pCoder->Encode(pstRecord);
    }
    
    fwrite(pstRecord, sizeof(tagRecord), 1, m_pFile);
    printf("save a record in %s\n", m_strFileName.c_str());
}

void PasswdOperator::Read(tagRecord* pstRecord)
{
    fread(pstRecord, sizeof(tagRecord), 1, m_pFile);
    
    if(m_pCoder)
    {
        m_pCoder->Decode(pstRecord);
    }
}

void PasswdOperator::Find(const string& filename, tagRecord* pstRecord)
{
    if(!pstRecord)
    {
        return;
    }
    if(!m_lRecordList.empty())
    {
        list<tagRecord>::iterator itor = m_lRecordList.begin();
        for(; itor != m_lRecordList.end(); ++itor)
        {
            if(itor->szKey == filename)
            {
                memcpy(pstRecord->szKey, itor->szKey, 126);
                pstRecord->szKey[127] = 0;
                memcpy(pstRecord->szValue, itor->szValue, 126);
                pstRecord->szValue[127] = 0;
                
                PrintRecord(*pstRecord);
                return;
            }
        }
    }

    fseek(m_pFile, 0, SEEK_END);
    long lPos = ftell(m_pFile);
    fseek(m_pFile, 0, SEEK_SET);

    for(; lPos != ftell(m_pFile); )
    {
        tagRecord r;
        Read(&r);
        if(string(r.szKey) == filename)
        {
            memcpy(pstRecord, &r, sizeof(tagRecord));
            PrintRecord(*pstRecord);
            break;
        }
    }
}

void PasswdOperator::FindAll()
{
    fseek(m_pFile, 0, SEEK_END);
    long lPos = ftell(m_pFile);
    fseek(m_pFile, 0, SEEK_SET);
    m_lRecordList.clear();

    for(; lPos != ftell(m_pFile); )
    {
        tagRecord r;
        Read(&r);
        m_lRecordList.push_back(r);
        PrintRecord(r);
    }
}

enum enInputParam
{
    QUERY,
    ADD
};

inline void PrintUsage()
{
    cout<<"Usage:cmd query[add] filename key value"<<endl;
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        PrintUsage();
        return 0;
    }

    // argv[2] 文件名，argv[3] 键值名
    
    PasswdOperator po(argv[2]);
    po.Init();
    tagRecord r;
    if(0 == strcmp(argv[1], "query") && argc >= 4)
    {
        if(0 == strcmp(argv[3], "all"))
        {
            po.FindAll();
        }
        else
        {
            po.Find(argv[3], &r);      
        }        
    }
    else if(0 ==strcmp(argv[1], "add") && argc >= 5)
    {
        int nLen = strlen(argv[3]);
        strncpy(r.szKey, argv[3], strlen(argv[3]));
        r.szKey[nLen] = 0;

        nLen = strlen(argv[4]);
        strncpy(r.szValue, argv[4], strlen(argv[4]));
        r.szValue[nLen] = 0;

        po.Write(&r);
    }
    else
    {
        cout<<"invalid command."<<endl;
    }

    return 0;
}
