//
//  common.h
//  NeuroEvolution
//
//  Created by Arthur Sun on 7/30/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef common_h
#define common_h

#include <cmath>
#include <functional>
#include <random>

namespace NE {
    
    typedef float float_t;
    
    inline uint32_t rand32() {
        return arc4random();
    }
    
    inline uint64_t rand64() {
        return rand32() | (uint64_t(rand32()) << 32);
    }
    
    inline void* malloc(size_t size) {
        return ::operator new(size);
    }
    
    inline void mfree(void* ptr) {
        ::operator delete(ptr);
    }
    
    inline float_t gaussian_randomf() {
        thread_local std::default_random_engine g;
        thread_local std::normal_distribution<float_t> d(0.0, 1.0);
        return d(g);
    }
    
    inline float_t randomf() {
        return rand32() / (float_t) (uint32_t(-1));
    }
    
    inline float_t randposneg() {
        return rand32() <= (uint32_t(-1) >> 1) ? float_t(-1) : float_t(1);
    }
    
    template <class T>
    struct list
    {
        struct type
        {
            T data;
            type* prev;
            type* next;
            
            type() {}
            
            type(const T& q) : data(q) {}
        };
        
        type* begin;
        size_t size;
        
        list() : begin(nullptr), size(0) {}
        
        list(const list<T>& l) : begin(nullptr), size(0) {
            *this = l;
        }
        
        list<T>& operator = (const list<T>& l) {
            clear();
            
            type* q = l.begin;
            
            while(q != nullptr) {
                insert(begin, q->data);
                q = q->next;
            }
        }
        
        ~list() {
            clear();
        }
        
        void clear() {
            while(begin != nullptr) {
                type* A = begin;
                begin = begin->next;
                delete A;
            }
            
            size = 0;
        }
        
        type* insert(type* A, type* ptr) {
            if(A == nullptr) A = begin;
            
            if(A == nullptr) {
                begin = ptr;
                begin->prev = nullptr;
                begin->next = nullptr;
            }else{
                if(A->prev != nullptr) {
                    A->prev->next = ptr;
                }
                
                ptr->next = A;
                ptr->prev = A->prev;
                A->prev = ptr;
                
                if(A == begin)
                    begin = ptr;
            }
            
            ++size;
            
            return ptr;
        }
        
        void remove(type* ptr) {
            if(ptr->prev == nullptr) {
                begin = ptr->next;
                
                if(begin != nullptr)
                    begin->prev = nullptr;
            }else{
                ptr->prev->next = ptr->next;
                
                if(ptr->next != nullptr)
                    ptr->next->prev = ptr->prev;
            }
            
            --size;
        }
    };
    
    enum function_types
    {
        e_linear = 0,
        e_elliot1,
        e_elliot2,
        e_gaussian,
        e_step1,
        e_step2,
        e_relu,
        e_abs,
        e_sine,
        e_cosine,
        functions_size
    };
    
    struct Function
    {
        int type;
        
        Function() {
            randomlize();
        }
        
        inline void randomlize() {
            type = rand32() % functions_size;
        }
        
        inline float_t operator () (float_t x) const {
            switch(type) {
                case e_linear:
                    return x;
                    
                case e_elliot1:
                    return 0.5f + (0.5f * x / (1.0f + fabs(x)));
                    
                case e_elliot2:
                    return x / (1.0f + fabs(x));
                    
                case e_gaussian:
                    return exp(-(x * x) * 0.5f);
                    
                case e_step1:
                    return x > 0.0f;
                    
                case e_step2:
                    return x > 0.0f ? 1.0f : -1.0f;
                    
                case e_relu:
                    return x > 0.0f ? x : 0.0f;
                    
                case e_abs:
                    return fabs(x);
                    
                case e_sine:
                    return sin(x);
                    
                case e_cosine:
                    return cos(x);
                    
                default:
                    return 0.0f;
            }
        }
    };
}

#endif /* common_h */
