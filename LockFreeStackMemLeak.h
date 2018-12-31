//
//  LockFreeStack
//
//  A simple lock-free stack that can potentially leak memory on pops.
//  guaranteed to work on any platform (because it uses
//  compare_exchange_weak() repeatedly till it can push/pop successfully
//
//
//  Created by Nandagopal, Harihar on 12/28/18.
//

#pragma once

#include <atomic>


template <typename T>
class LockFreeStackMemLeak
{
public:
    void push(T data)
    {
        // construct newNode
        StackNode* newNode = new StackNode(data);
        
        // set newNode->next to head
        newNode->next = head.load();
        
        // CASWeak head from newNode->next to newNode
        // if it passes, head will be set to newNode
        // if it fails, newNode->next will be set to head.load() and we repeat!
        while(!head.compare_exchange_weak(newNode->next, newNode));
    }
    
    T pop()
    {
        // set oldHead to head
        StackNode* oldHead = head.load();
        
        // CASWeak head from oldHead to oldHead->next
        // if it passes, head is set to oldHead->next
        // if it fails, oldHead is set to head.load() which
        // could now be nullptr, so check if its non-null
        while(oldHead && !head.compare_exchange_weak(oldHead, oldHead->next));

        // *** cannot delete oldHead here because another thread could be
        // doing CAS with the same ptr at this very moment!
        // Hence this pop() design leaks memory!
        
        // if its null, then the stack was empty when we tried to pop
        return (oldHead? oldHead->data : T());
    }
    
private:
    struct StackNode
    {
        T data;
        StackNode* next;
        
        StackNode(inData) : data(inData), next(nullptr) {}
    };

    std::atomic<StackNode*> head;
};