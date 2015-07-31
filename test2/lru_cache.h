#ifndef _LRU_CACHE_H_
#define _LRU_CACHE_H_
#include <iostream>
#include <list>

template <typename T>
class LRUCache{
public:
    typedef struct CacheNode
	{
        int key;
        T value;
        CacheNode(int key, T value):key(key), value(value){}
    }cache_node_t;
	
    LRUCache(unsigned int cache_size);
    ~LRUCache();
    bool put(int key, T value);
    bool get(int key, T *value);
    void dsp_nodes();
    size_t Size();
private:
    bool del_last();
    bool put_to_first(cache_node_t node);
private:
    typename std::list<cache_node_t> m_List;
    unsigned int m_CacheSize;
};

template <typename T>
LRUCache<T>::LRUCache(unsigned int cache_size):m_CacheSize(cache_size)
{
}

template <typename T>
LRUCache<T>::~LRUCache(){}

template <typename T>
bool LRUCache<T>::get(int key, T *value)
{
    if(m_List.empty()){
        std::cout<<"cache is empty"<<std::endl;
        return false;
    }
    typename std::list<cache_node_t>::iterator itor = m_List.begin();
    for(; itor != m_List.end(); ++itor){
        if((*itor).key == key){
            *value = (*itor).value;
            cache_node_t node((*itor).key, (*itor).value);
            m_List.erase(itor);
            m_List.push_front(node);
            return true;
        }
    }
    return false;
}

template <typename T>
bool LRUCache<T>::put(int key, T value)
{
    if(m_CacheSize <= 0){
        std::cout<<"cache size is less than 0"<<std::endl;
        return false;
    }

    bool ret = false;
    if(m_List.size() >= m_CacheSize){

        //删掉最后一个
        ret = del_last();
        if(!ret)
            return false;
    }
    cache_node_t node(key, value);

    //放在最前面
    ret = put_to_first(node);
    return ret;
}

template <typename T>
bool LRUCache<T>::del_last(){
    if(m_List.empty()){
        std::cout<<"cache is empty, can not delete element"<<std::endl;
        return false;
    }
    std::cout<<"delete last"<<std::endl;
    m_List.pop_back();
    return true;
}

template <typename T>
bool LRUCache<T>::put_to_first(cache_node_t node){
    m_List.push_front(node);
    return true;
}

template <typename T>
void LRUCache<T>::dsp_nodes()
{
    if(m_List.empty()){
        std::cout<<"cache is empty, nothing to display"<<std::endl;
        return;
    }
    typename std::list<cache_node_t>::iterator itor = m_List.begin();
    while(itor != m_List.end()){
        std::cout<<(*itor).value<<std::endl;
        ++itor;
    }
    std::cout<<""<<std::endl;
}

template <typename T>
size_t LRUCache<T>::Size()
{
    return m_List.size();
}
#endif
