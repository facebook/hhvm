#pragma once

#include <stdio.h>

namespace HPHP { namespace hfsort {

void error(const char* msg);
int jitsort(int pid, int time, FILE* perfSymFile, FILE* relocResultsFile);

} }
