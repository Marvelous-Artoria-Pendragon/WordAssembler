/*HashTable 结构体*/
#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <assert.h>
const int defaultSize = 256;
template <class E, class K>
struct ChainNode                    // 各桶中同义词子表的链结点定义
{
    E data;                         // 元素
    ChainNode<E, K> *link;          // 链指针
    ChainNode(ChainNode<E, K> *p = NULL) {link = p;}
    ChainNode(const E &elem, ChainNode<E, K> *p = NULL) {data = elem; link = p;}
};

template <class E, class K>
class HashTable                     // 散列表定义
{
    public:
        HashTable(int d, int sz = defaultSize);                   // 散列表构造函数
        ~HashTable();                                             // 析构函数
        bool search(const K key, E &elem);                        // 搜索
        bool insert(const K key, E &elem);                        // 插入
        // bool remove(const K key, E &elem);                        // 删除
        ChainNode<E, K> *findPos(const K key);                                // 寻找链结点地址
    private:
        int divisor;                // 除数
        int TableSize;              // 桶的容量
        ChainNode<E, K> *ht;        // 散列表定义，带表头结点的拉链
        ChainNode<E, K> *FindPos(const K key);       //散列
};

template <class E, class K>
HashTable<E, K>::HashTable(int d, int sz)
{
    divisor = d; TableSize = sz;
    ht = new ChainNode<E, K> [sz];             // 创建头结点
    assert(ht != NULL);                         // 判断存储分配成功与否
}

template <class E, class K>
HashTable<E, K>::~HashTable()
{
    for (int i = 0; i < TableSize; i++)
    {
        ChainNode<E, K> *p = ht[i].link, *del = p;
        while (p != NULL) {p = p->link; delete del; del = p;}       // 释放拉链
    } delete []ht;
}

template <class E, class K>
bool HashTable<E, K>::search(const K key, E &elem)
{
    int j = key % divisor;
    ChainNode<E, K> *p = ht[j].link;
    while (p != NULL && p->data != elem) p = p->link;
    if (p != NULL) {elem = p->data; return true;}           // 寻找成功
    else return false;
}

template <class E, class K>
bool HashTable<E, K>::insert(const K key, E &elem)
{
    int j = key % divisor;
    ChainNode<E, K> *p = &(ht[j]);
    while (p->link != NULL) p = p->link;                  // 冲突，寻找空结点
    p->link = new ChainNode<E, K>(elem);
    if (p->link != NULL) return true;
    return false;
}

template <class E, class K>
ChainNode<E, K> *HashTable<E, K>::findPos(const K key)
{
    int j = key % divisor;                      // 计算散列地址
    ChainNode<E, K> *p = ht[j].link;                 // 扫描第j链的同义词字表
    while (p != NULL && p->data != key) p = p->link;
    return p;                                   // 返回
}
#endif //HASHTABLE_H
