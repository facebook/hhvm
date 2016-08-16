#ifndef incl_HPHP_FACEBOOK_HFSORT_JITSORT_H
#define incl_HPHP_FACEBOOK_HFSORT_JITSORT_H

#include <stdio.h>

namespace HPHP { namespace hfsort {

void error(const char* msg);
int jitsort(int pid, int time, FILE* perfSymFile, FILE* relocResultsFile);

} }

#endif
