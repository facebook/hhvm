/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EVAL_CODE_COVERAGE_H_
#define incl_HPHP_EVAL_CODE_COVERAGE_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/util/lock.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class CodeCoverage {
public:
  void Record(const char *filename, int line0, int line1);

  /**
   * Returns an array in this format,
   *
   *  array('filename' => array( line => count, ...))
   */
  Array Report();

  /**
   * Write JSON format into the file.
   *
   *  { 'filename': [0, 0, 1, 0, 2, 0], ...}
   *
   * Note it's 0-indexed, so first count should always be 0.
   */
  void Report(const std::string &filename);

  /**
   * Clear all coverage data.
   */
  void Reset();

private:
  typedef hphp_const_char_map<std::vector<int> > CodeCoverageMap;
  CodeCoverageMap m_hits;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EVAL_CODE_COVERAGE_H_
