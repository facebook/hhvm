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

#pragma once

#include "hphp/runtime/base/req-hash-map.h"
#include "hphp/runtime/base/req-bitset.h"

#include "hphp/util/optional.h"

#include <string>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Array;

struct CodeCoverage {
  void Record(const char* filename, int line);
  void onSessionInit();
  void onSessionExit();

  /*
   * If report_frequency is passed, returns an array in this format,
   *
   * array('filename' => covered_line_count, ....)
   *
   * Otherwise, returns an array in this format,
   *
   *  array('filename' => array( line => count, ...))
   *
   * If sys is passed as false, systemlib files are not included.
   */
  Array Report(bool report_frequency = false, bool sys = true);

  /*
   * Write JSON format into the file.
   *
   *  { 'filename': [0, 0, 1, 0, 2, 0], ...}
   *
   * Note it's 0-indexed, so first count should always be 0.
   */
  void Report(const std::string& filename);

  /*
   * Clear all coverage data.
   */
  void Reset();

  /*
   * Causes CodeCoverage to dump any coverage data onSessionExit()
   */
  void dumpOnExit() { shouldDump = true; }

  /*
   * Whether or not coverage should use per file coverage
   */
  bool m_should_use_per_file_coverage{false};

private:
  using CodeCoverageMap = req::vector_map<const char*, req::dynamic_bitset>;
  Optional<CodeCoverageMap> m_hits;
  bool shouldDump{false};
};

///////////////////////////////////////////////////////////////////////////////
}
