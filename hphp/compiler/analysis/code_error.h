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

#ifndef incl_HPHP_COMPILER_ERROR_H_
#define incl_HPHP_COMPILER_ERROR_H_

#include "hphp/compiler/analysis/type.h"
#include "hphp/util/json.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(Construct);

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

enum ErrorType {
#define CODE_ERROR_ENTRY(x) x,
#include "hphp/compiler/analysis/core_code_error.inc"
#undef CODE_ERROR_ENTRY
  ErrorCount,
  NoError
};

/**
 * Call this before analysis.
 */
void ClearErrors();

/**
 * Record a coding error.
 */
void Error(ErrorType error, ConstructPtr construct);

/**
 * Record a coding error with an extra construct information.
 */
void Error(ErrorType error, ConstructPtr construct1, ConstructPtr construct2);

/**
 * Record a coding error with an extra string.
 */
void Error(ErrorType error, ConstructPtr construct, const std::string &data);

/**
 * Save JavaScript output to specified file.
 */
void SaveErrors(JSON::CodeError::OutputStream &out);
void SaveErrors(AnalysisResultPtr ar,
                const char *filename,
                bool varWrapper = false);

/**
 * Write errors to stderr.
 */
void DumpErrors(AnalysisResultPtr ar); // stderr

/**
 * Whether specified type of error is present. Written for unit test.
 */
bool HasError(ErrorType type);
bool HasError(); // any error

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_COMPILER_ERROR_H_
