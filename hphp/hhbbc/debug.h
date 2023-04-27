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

#include "hphp/util/trace.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-structure.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

struct ParsedUnit;

//////////////////////////////////////////////////////////////////////

/*
 * If the hhbbc_dump trace module is on, dump the entire program to a
 * temporary directory as readable text.
 */
std::string debug_dump_to();
void dump_representation(const std::string& dir,
                         const Index&,
                         const php::Unit&);
void dump_index(const std::string&, const Index&, const php::Unit&);

/*
 * Utilities for printing the state of the program after various
 * transformations.
 */

void state_after(const char* when, const php::Unit&, const Index&);
void state_after(const char* when, const ParsedUnit&);

//////////////////////////////////////////////////////////////////////

}
