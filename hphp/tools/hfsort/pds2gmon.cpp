/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define BUFLEN 1000

struct GmonHeader {
  char cookie[4];
  char version[4];
  char spare[12];
};

struct GmonArc {
  uint64_t callerAddr;  // address within caller's body
  uint64_t calleeAddr;  // address within callee's body
  uint32_t count;       // number of arc traversals
};

void error(const char* msg) {
  printf("ERROR: %s\n", msg);
  exit(1);
}

void writeGmonHeader(FILE* outFile) {
  fwrite("gmon", 4, 1, outFile);
  uint32_t version = 1;
  fwrite(&version, 4, 1, outFile);
  uint32_t spare[3] = {0, 0, 0};
  fwrite(&spare, 4, 3, outFile);
}

void processPerfData(gzFile inFile, FILE* outFile) {
  char line[BUFLEN];
  uint8_t  tag = 1; // GMON_TAG_CG_ARC
  uint64_t calleeAddr;
  uint64_t callerAddr;
  struct GmonArc arc;

  while (gzgets(inFile, line, BUFLEN) != Z_NULL) {
    if (line[0] == '#') continue;
    if (isspace(line[0])) continue;

    // process one sample
    if (gzgets(inFile, line, BUFLEN) == Z_NULL) error("reading perf data");
    if (sscanf(line, "%" SCNx64, &calleeAddr) != 1) continue;
    if (gzgets(inFile, line, BUFLEN) == Z_NULL) error("reading perf data");
    if (sscanf(line, "%" SCNx64, &callerAddr) == 1) {
      fwrite(&tag, 1, 1, outFile);
      arc.calleeAddr = calleeAddr;
      arc.callerAddr = callerAddr;
      arc.count = 1;
      fwrite(&arc, 20, 1, outFile);
    }
  }
}

void usage() {
  fprintf(stderr, "Usage: pds2gmon <PERF_DATA_FILE> <OUTPUT_FILE>\n");
}

int main(int argc, char* argv[]) {

  if (argc != 3) {
    usage();
    exit(1);
  }

  char *inFileName  = argv[1];
  char *outFileName = argv[2];

  gzFile inFile;
  if (!(inFile = gzopen(inFileName, "r"))) error("opening input file");

  FILE* outFile = fopen(outFileName, "wt");
  if (!outFile) error("opening output file");

  writeGmonHeader(outFile);

  processPerfData(inFile, outFile);

  gzclose(inFile);
  fclose(outFile);

  return 0;
}
