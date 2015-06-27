#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

template<typename T>
struct tagRecord
{
	T a;
	T b;
};

typedef tagRecord<string> StringRecord;

int main()
{
	FILE* fp = fopen("coredump_notes", "a+");

	tagRecord<string> record;
	for(int i = 0; i < 2; ++i)
	{
		record.a = "hello";
		record.b = "world";
		fwrite(&record, sizeof(record), 1, fp);
		printf("save:%s---%s\n", record.a.c_str(), record.b.c_str());
	}

	fclose(fp);
	fp = NULL;

	cout<<" "<<endl;

	fp = fopen("coredump_notes", "a+");

    fseek(fp, 0, SEEK_END);
	long pos = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	for(; pos != ftell(fp); )
	{
		fread(&record, sizeof(record), 1, fp);

		// 发生coredump，string的真正存储在堆中，从文件中读取的指针已经不可再用
		printf("read:%s---%s\n", record.a.c_str(), record.b.c_str());
	}


	fclose(fp);
	fp = NULL;
	return 0;
}

