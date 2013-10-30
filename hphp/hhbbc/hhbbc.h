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
#ifndef incl_HPHP_HHBBC_H_
#define incl_HPHP_HHBBC_H_

#include <vector>
#include <memory>
#include <string>
#include <set>

namespace HPHP { struct UnitEmitter; }
namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * This is the public API to this subsystem.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Publically-settable options that control compilation.
 */
struct Options {
  /*
   * Functions that we should assume may be used with fb_intercept.
   * Functions that aren't named in this list may be optimized with
   * the assumption they aren't intercepted, in whole_program mode.
   */
  std::set<std::string> InterceptableFunctions;
};

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
std::vector<std::unique_ptr<UnitEmitter>>
whole_program(std::vector<std::unique_ptr<UnitEmitter>>, const Options&);

/*
 * Perform single-unit optimizations.
 */
std::unique_ptr<UnitEmitter> single_unit(std::unique_ptr<UnitEmitter>);

//////////////////////////////////////////////////////////////////////

}}

#endif
