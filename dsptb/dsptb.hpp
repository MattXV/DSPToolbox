#ifndef DSPTB_HPP
#define DSPTB_HPP
#pragma once

#include <dsptb.h>
#include "ir.hpp"
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <array>

namespace dsptb {
    std::unique_ptr<FilterBank> filterBank;

    extern std::string currentLogs;
    bool dsptbInitOK = false;
}

#endif