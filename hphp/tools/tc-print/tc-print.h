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

#ifndef incl_HPHP_TCPRINT_H_
#define incl_HPHP_TCPRINT_H_

#include "hphp/tools/tc-print/annotation-cache.h"
#include "hphp/tools/tc-print/offline-trans-data.h"
#include "hphp/tools/tc-print/repo-wrapper.h"
#include "hphp/tools/tc-print/tc-print-logger.h"

#include <folly/Format.h>

#include <iostream>
#include <string>

template<typename... Args>
[[noreturn]] void error(Args&&... args) {
  std::cerr << "Error: " << folly::format(std::forward<Args>(args)...) << '\n';
  exit(1);
}

extern HPHP::jit::RepoWrapper* g_repo;
extern HPHP::jit::OfflineTransData* g_transData;
extern HPHP::TCPrintLogger* g_logger;
extern std::unique_ptr<HPHP::AnnotationCache> g_annotations;

#endif
