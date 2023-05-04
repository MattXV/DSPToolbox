#ifndef DSPTB_HPP
#define DSPTB_HPP
#pragma once

#include <dsptb.h>
#include "ir.hpp"
#include "processing.hpp"
#include <memory>
#include <string>
#include <cstring>
#include <sstream>
#include <map>


namespace dsptb {
    std::unique_ptr<FilterBank> filterBank;
    
    std::map<int, std::unique_ptr<OverlapAdd>> blockProcessingObjects;
    int blockProcessingCounter = 0;
    std::map<int, std::unique_ptr<HRTF>> hrtfObjects;
    int hrtfCounter = 0;

    // Data
    signal outConvolution;

    extern std::string currentLogs;
    bool dsptbInitOK = false;

}

#endif