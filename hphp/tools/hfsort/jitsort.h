#pragma once

#include <stdio.h>

namespace HPHP { namespace hfsort {

[[noreturn]] void error(const char* msg);
int jitsort(int pid, int time, FILE* perfSymFile, FILE* relocResultsFile);

} }
