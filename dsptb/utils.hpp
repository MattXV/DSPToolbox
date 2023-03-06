#ifndef DSPTB_UTILS_HPP
#define DSPTB_UTILS_HPP
#pragma once

#include <dsptb.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <random>
#include <utility>

#define DISABLE_COPY_ASSIGN(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete;

#ifndef NDEBUG
    #define DSPTB_ERROR(String) do { DSPTB_SetError(String); } while (0)
    #define DSPTB_LOG(Message)  do { std::stringstream log; log << Message; std::cout << log.str() << std::endl; DSPTB_Log(log.str().c_str()); } while (0)
#else
    #define DSPTB_ERROR(Message)    do {} while (0)
    #define DSPTB_LOG(Message)      do {} while (0)

#endif



void DSPTB_SetError(const std::string& error_string);
void DSPTB_Log(const std::string& log);

namespace dsptb {

    extern dsptbSETTINGS settings;
    typedef float sample;
    typedef std::vector<sample> signal;
    constexpr float pi = 3.141592653589793f;
    constexpr float speed_of_sound = 343.0f;

    static inline float clip( float n, float lower, float upper )
    {
        if (std::isnan(n)) n = 0;
    n = ( n > lower ) * n + !( n > lower ) * lower;
    return ( n < upper ) * n + !( n < upper ) * upper;
    }   

    inline unsigned int roundUpToNextPowerOfTwo(unsigned int x)
    {
        x--;
        x |= x >> 1;  // handle  2 bit numbers
        x |= x >> 2;  // handle  4 bit numbers
        x |= x >> 4;  // handle  8 bit numbers
        x |= x >> 8;  // handle 16 bit numbers
        x |= x >> 16; // handle 32 bit numbers
        x++;
        return x;
    }

    template<typename T>
    T random(T range_from, T range_to) {
        std::random_device                  rand_dev;
        std::mt19937                        generator(rand_dev());
        std::uniform_int_distribution<T>    distr(range_from, range_to);
        return distr(generator);
    }

}
#endif