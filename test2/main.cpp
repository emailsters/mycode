#include <iostream>
#include "lru_cache.h"
using namespace std;
int main()
{
	LRUCache<int> lruCache(1000);
	for(int i = 0; i < 10000; ++i)
	{
		lruCache.put(i, i);
	}
	cout<<lruCache.Size()<<endl;
    lruCache.dsp_nodes();
	return 0;
}