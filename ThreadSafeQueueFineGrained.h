//
//  ThreadSafeQueueFineGrained.h
//
//  fine grained thread-safe queue that implements the queue as a linked list
//  with head and tail pointers.
//  i) start with a dummy head/tail, after 1 push the head points to the new value
//  and tail points to the dummy node
//  ii) thread locking is fine grained to only protecting access to the head and
//  tail nodes and even further only protect the pointer swapping (ie: not the allocations
//  /writing of value, etc.
//  iii) this implementation is more serializable (ie: threads can push and pop at
//  the same time, 2 or more pushing threads can allocate memory concurrently, 2
//  popping threads can deallocate memory/write result concurrently
//
//  Design is based off the tutorial in Anthony Williams' C++ Concurrency In Action
//  book (Chapter 6).- https://www.oreilly.com/library/view/c-concurrency-in/9781933988771/
//
//  Created by Nandagopal, Harihar on 12/25/18.
//

#pragma once

#include <mutex>

template<typename T>
class ThreadSafeQueueFineGrained
{
public:
    ThreadSafeQueueFineGrained() : head(new Node(T())), tail(head) {}
    
    
    void push(T newVal)
    {
        // keep allocation outside the lock - HUGE win!
        Node* newTail = new Node(T());
        std::lock_guard<std::mutex> lg(tailMutex);
        tail->data = newVal;
        tail->next = newTail;
        tail = newTail;
    }
    
    bool tryPop(T& value)
    {
        // unique lock needed for unlock
        std::unique_lock<std::mutex> ul(headMutex);
        
        // call to getTail() is done here as against before locking
        // headMutex because otherwise this thread can be preempted
        // after getTail() and before locking headMutex by another
        // popping/pushing thread, and the tail we initially got can
        // no longer be valid.
        //
        if (head == getTail())
            return false;
        else
        {
            Node* temp = head;
            head = head->next;
            ul.unlock(); // lock is not needed after this, so unlock!
            value = temp->data;
            delete temp;
            return true;
        }
    }
    
private:
    struct Node
    {
        T data;
        Node* next;
        
        Node(T _data) : data(_data), next(nullptr) {}
    };
    
    Node* getTail()
    {
        std::lock_guard<std::mutex> lg(tailMutex);
        return tail;
    }
    
    Node* head;
    Node* tail;
    mutable std::mutex headMutex;
    mutable std::mutex tailMutex;
};
