#pragma once

#ifndef BOARD_H
#define BOARD_H

#include <utility>

#include "types.h"

std::pair<bool, std::string> validateFen(std::string fen);

#endif
