//
//  UnionFind.h
//  hnandag2/data-structs
//
//  C++ implementation of the really cool UnionFind class that I learned about from
//  watching William Fiset's brilliant Youtube tutorial on this data structure -
//  https://www.youtube.com/watch?v=ibjEGG7ylHk
//
//  TODO: implement Path Compression in the find() function
//  TODO: implement working Kruskal's algorithm example using a UnionFind object
//
//  Created by Nandagopal, Harihar on 12/23/18.
//

#include <iostream>
#include <vector>
#include <unordered_map>



template <typename T>
class UnionFind
{
public:
    UnionFind()
    {
        
    }
    
    UnionFind(std::vector<T> elements)
    {
        for (size_t i = 0; i < elements.size(); i++)
        {
            rootMap.insert(std::pair<T,T>(elements[i], elements[i]));
            sizeMap.insert(std::pair<T,int>(elements[i], 1));
        }
    }
    
    void insert(T element)
    {
        rootMap.insert(std::pair<T,T>(element, element));
        sizeMap.insert(std::pair<T,int>(element, 1));
    }
    
    T find(const T& element)
    {
        T root = element;
        
        if (rootMap.find(element) != rootMap.end())
        {
            while (rootMap[root] != root)
            {
                root = rootMap[root];
            }
        }
                   
        return root;
    }
    
    void unionize(const T& element1, const T& element2)
    {
        T root1 = find(element1);
        T root2 = find(element2);
        
        if (root1 == root2)
        {
            return;
        }
        else
        {
            if (sizeMap[root1] >= sizeMap[root2])
            {
                rootMap[root2] = root1;
                sizeMap[root1] += sizeMap[root2];
            }
            else
            {
                rootMap[root1] = root2;
                sizeMap[root2] += sizeMap[root1];
            }
        }
    }
    
    void printRootMap()
    {
        for (auto it = rootMap.begin(); it != rootMap.end(); it++)
        {
            std::cout << it->first << " " << it->second << std::endl;
        }
    }
    
private:
    std::unordered_map<T, T> rootMap;
    std::unordered_map<T, int> sizeMap;
};
