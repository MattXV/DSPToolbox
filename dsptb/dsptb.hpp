#ifndef DSPTB_HPP
#define DSPTB_HPP
#pragma once

#include <dsptb.h>
#include "ir.hpp"
#include "processing.hpp"
#include <memory>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <array>

namespace dsptb {
    std::unique_ptr<FilterBank> filterBank;
    
    std::unique_ptr<OverlapSave> overlapSave;

    // Data
    signal outConvolution;

    extern std::string currentLogs;
    bool dsptbInitOK = false;

}

#endif