/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/maphuge.h"

#include "hphp/util/assertions.h"
#include <stdio.h>

namespace HPHP {

static size_t s_thpPageSize = 0;

size_t readTHPSize() {
  if (s_thpPageSize) return s_thpPageSize;
  FILE* fp = fopen("/sys/kernel/mm/transparent_hugepage/hpage_pmd_size", "r");
  if (fp) {
    fscanf(fp, "%zu", &s_thpPageSize);
    fclose(fp);
    // Page size should be a power of two.
    always_assert(!(s_thpPageSize & (s_thpPageSize - 1)));
  }
  return s_thpPageSize;
}

}
