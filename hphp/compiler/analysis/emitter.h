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

#include <memory>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

struct AnalysisResult;
struct SHA1;
struct Unit;
struct RepoOptions;

namespace Native {
struct FuncTable;
}

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

void emitAllHHBC(std::shared_ptr<AnalysisResult>&&);

extern "C" {
  Unit* hphp_compiler_parse(const char* code, int codeLen, const SHA1& sha1,
                            const char* filename,
                            const Native::FuncTable& nativeFuncs,
                            Unit** releaseUnit,
                            bool forDebuggerEval,
                            const RepoOptions&);
}

///////////////////////////////////////////////////////////////////////////////
}}

