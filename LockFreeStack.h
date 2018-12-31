//
//  LockFreeStack
//
//  A simple lock-free stack that garbage collects popped nodes. Garbage
//  collection is triggered by a pop(). All leaked nodes are guaranteed
//  to be garbage collected at the last pop()- since the last pop() will
//  be by a single thread.
//
//  Note: compare_exchange_weak() is used to improve efficiency on
//  platforms that do not guarantee single instruction CAS.
//
//
//  Created by Nandagopal, Harihar on 12/28/18.
//

#pragma once

#include <atomic>


template <typename T>
class LockFreeStack
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
        // if it fails, newNode->next will be set to the new head
        // and we repeat the CAS
        while(!head.compare_exchange_weak(newNode->next, newNode));
    }
    
    T pop()
    {
        ++poppingThreads;
        
        // set oldHead to head
        StackNode* oldHead = head.load();
        
        // CASWeak head from oldHead to oldHead->next
        // if it passes, head is set to oldHead->next
        // if it fails, oldHead is set to the new head
        // which could now be nullptr, so check if its non-null
        // and if not, repeat the CAS
        while(oldHead && !head.compare_exchange_weak(oldHead, oldHead->next));

        // if its null, then the stack was empty when we tried to pop
        T ret = oldHead? oldHead->data : T();
        
        // garbage collect the popped node
        reclaim(oldHead);
        
        return res;
    }
    
private:
    // garbage collect popped nodes
    void reclaim(StackNode* node)
    {
        // if this is the only thread that has popped
        if (poppingThreads.load() == 1)
        {
            // safe to delete node
            delete node;
            
            // also safe to delete the list of pending nodes
            // this thread has now claimed the list, other threads will
            // get nullptr if they access/add to the list
            StackNode* delNodes = toBeDeleted.exchange(nullptr);
            
            // 2nd check to see if we are the only thread popping, then safe to delete them
            if (--poppingThreads == 0)
            {
                // safe to delete list
                while (delNodes)
                {
                    StackNode* temp = delNodes->next;
                    delete delNodes;
                    delNodes = temp;
                }
            }
            else
            {
                // not safe to delete list since other threads may be accessing it
                // chain it back
                chainPendingNodes(delNodes);
            }
            
        }
        else
        {
            // not safe to delete node, since other threads may be accessing it
            // chain it back
            chainPendingNodes(node);
            --poppingThreads;
        }
    }
    
    // go to end of node list and chain it to toBeDeleted chain
    void chainPendingNodes(StackNode* node)
    {
        // go to the last node in the list
        Node* last = node;
        while (last->next != nullptr)
        {
            last = last->next;
        }
        
        // chain last to toBeDeleted and CAS toBeDeleted to first
        last->next = toBeDeleted;
        
        // CASWeak toBeDeleted to node
        // If it passes, toBeDeleted is set to node (we are done!)
        // If it fails, last->next is set to the new toBeDeleted
        // and we repeat the CAS
        while (!toBeDeleted.compare_exchange_weak(last->next, node));
        
    }
    
    struct StackNode
    {
        T data;
        StackNode* next;
        
        StackNode(inData) : data(inData), next(nullptr) {}
    };

    std::atomic<StackNode*> head;
    std::atomic<size_t> poppingThreads{0};
    std::atomic<StackNode*> toBeDeleted;
};