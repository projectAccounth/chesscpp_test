#pragma once

#ifndef STRFCNS_H
#define STRFCNS_H

#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <regex>

std::vector<std::string> split(const std::string& str, char delimiter);

std::vector<std::string> splitWithRegex(const std::string& str, const std::string& pattern);

std::string join(const std::vector<std::string>& elements, const std::string& delimiter);

bool isDigit(std::string c);

#endif