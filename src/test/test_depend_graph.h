/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __TEST_DEPEND_GRAPH_H__
#define __TEST_DEPEND_GRAPH_H__

#include <test/test_base.h>
#include <compiler/analysis/dependency_graph.h>

///////////////////////////////////////////////////////////////////////////////

class TestDependGraph : public TestBase {
 public:
  virtual bool RunTests(const std::string &which);

#define DEPENDENCY_ENTRY(x) bool Test ## x();
#include "../compiler/analysis/core_dependency.inc"
#undef DEPENDENCY_ENTRY

 private:
  bool VerifyDependency(HPHP::DependencyGraph::KindOf kindOf,
                        const char *input1, const char *input2 = NULL,
                        const char *parent = "f2",
                        const char *child = "f1",
                        const char *file = "",
                        int line = 0);
};

///////////////////////////////////////////////////////////////////////////////
// macros

#define VD1(k,a)                                                        \
  if (!Count(VerifyDependency(DependencyGraph::KindOf ## k, a, NULL,    \
                              "f2", "f1", __FILE__, __LINE__)))         \
    return false;                                                       \

#define VD2(k,a,b)                                                      \
  if (!Count(VerifyDependency(DependencyGraph::KindOf ## k, a, b,       \
                              "f2", "f1", __FILE__, __LINE__)))         \
    return false;                                                       \

#define VD4(k,a,b,p,c)                                                  \
  if (!Count(VerifyDependency(DependencyGraph::KindOf ## k, a, b,       \
                              p, c, __FILE__, __LINE__)))               \
    return false;                                                       \

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_DEPEND_GRAPH_H__
