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
#ifndef incl_HPHP_HHBBC_H_
#define incl_HPHP_HHBBC_H_

#include <atomic>
#include <deque>
#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "hphp/hhbbc/options.h"

#include "hphp/runtime/base/repo-auth-type-array.h"

#include "hphp/util/lock.h"
#include "hphp/util/synchronizable.h"

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

template<class SinglePassReadableRange>
OpcodeSet make_bytecode_map(SinglePassReadableRange& bcs);

//////////////////////////////////////////////////////////////////////

struct UnitEmitterQueue : Synchronizable {
  // Add another ue. Adding nullptr marks us done.
  void push(std::unique_ptr<UnitEmitter> ue);
  // Get the next ue, or nullptr to indicate we're done.
  std::unique_ptr<UnitEmitter> pop();
  // Fetch any remaining ues.
  // Must be called in single threaded mode, after we've stopped adding ues.
  void fetch(std::vector<std::unique_ptr<UnitEmitter>>& ues);
  // Clear done flag, and get us ready for reuse.
  void reset();
 private:
  std::deque<std::unique_ptr<UnitEmitter>> m_ues;
  std::atomic<bool> m_done{};
};

/*
 * Perform whole-program optimization on a set of UnitEmitters.
 *
 * Currently this process relies on some information from HPHPc.  It
 * expects traits are already flattened (it might be wrong if they
 * aren't).
 */
std::unique_ptr<ArrayTypeTable::Builder>
whole_program(std::vector<std::unique_ptr<UnitEmitter>>,
              UnitEmitterQueue& ueq,
              int num_threads = 0);

//////////////////////////////////////////////////////////////////////

/*
 * Main entry point when the program should behave like hhbbc.
 */
int main(int argc, char** argv);

//////////////////////////////////////////////////////////////////////

}}

#endif
