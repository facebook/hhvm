/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_HHBBC_H_
#define incl_HPHP_HHBBC_H_

#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "hphp/hhbbc/options.h"

#include "hphp/runtime/base/repo-auth-type-array.h"

namespace HPHP { struct UnitEmitter; }
namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * This is the public API to this subsystem.
 */

//////////////////////////////////////////////////////////////////////

// Create a method map for the options structure from a SinglePassReadableRange
// containing a list of Class::methodName strings.
template<class SinglePassReadableRange>
MethodMap make_method_map(SinglePassReadableRange&);

//////////////////////////////////////////////////////////////////////

/*
 * Perform whole-program optimization on a set of UnitEmitters.
 *
 * Currently this process relies on some information from HPHPc.  It
 * expects AttrUnique/AttrPersistent have already been set up
 * correctly (but won't be wrong if they aren't set up at all), and
 * expects traits are already flattened (it might be wrong if they
 * aren't).
 */
std::pair<
  std::vector<std::unique_ptr<UnitEmitter>>,
  std::unique_ptr<ArrayTypeTable::Builder>
>
whole_program(std::vector<std::unique_ptr<UnitEmitter>>);

//////////////////////////////////////////////////////////////////////

/*
 * Main entry point when the program should behave like hhbbc.
 */
int main(int argc, char** argv);

//////////////////////////////////////////////////////////////////////

}}

#include "hphp/hhbbc/hhbbc-inl.h"

#endif
