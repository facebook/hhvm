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

#ifndef incl_HPHP_COMPILER_ANALYSIS_LAMBDA_NAMES_H_
#define incl_HPHP_COMPILER_ANALYSIS_LAMBDA_NAMES_H_

#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/analysis_result.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * After the first analysis pass over a file, this pass must run to
 * resolve names for lambda expressions and determine their automatic
 * capture lists.
 */
void resolve_lambda_names(AnalysisResultPtr ar, const FileScopePtr&);

//////////////////////////////////////////////////////////////////////

}

#endif
