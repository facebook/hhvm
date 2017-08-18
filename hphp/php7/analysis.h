/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_HPHP_PHP7_ANALYSIS_H
#define incl_HPHP_PHP7_ANALYSIS_H

#include "hphp/php7/unit.h"

#include <unordered_set>

namespace HPHP { namespace php7 {

/* Find all named locals and allocate IDs for unnamed locals */
std::unordered_set<std::string> analyzeLocals(const Function& func);

/* Find the number of classref slots needed to execute this CFG and allocate
 * classref IDs */
uint32_t analyzeClassrefs(const CFG& cfg);

/* Simplifies the given CFG, coalescing blocks to remove redundant labels */
void simplifyCFG(CFG& cfg);

}} // namespace HPHP::php7

#endif // incl_HPHP_PHP7_ANALYSIS_H
