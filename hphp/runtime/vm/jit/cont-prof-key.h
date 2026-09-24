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

#include <compare>
#include <optional>
#include <string>

#include "hphp/util/sha1.h"

namespace HPHP {

struct Func;
struct Unit;

}

namespace HPHP::jit {

/*
 * A restart-stable identity for a Hack function.
 *
 * `resolutionUnitPath' is the unit that must be loaded to look the function
 * up (for a method, the unit declaring the class). `bytecodeUnitPath' is set
 * only when the bytecode lives in a different unit, e.g. a trait method
 * flattened into a class. Paths are SourceRoot-relative and canonical.
 *
 * Builtins are never keyed.
 */
struct ContProfFuncKey {
  std::string resolutionUnitPath;
  std::optional<std::string> bytecodeUnitPath;
  SHA1 bytecodeUnitHash{};

  std::string functionName;
  std::optional<std::string> className;
  std::optional<std::string> closureContextName;

  std::strong_ordering operator<=>(const ContProfFuncKey&) const = default;
};

bool isValidContProfFuncKey(const ContProfFuncKey&);

/*
 * Build a key for `func', or nullopt if it can't be keyed stably
 * (builtin, unit outside SourceRoot, missing bytecode hash).
 */
std::optional<ContProfFuncKey> makeContProfFuncKey(const Func&);

/*
 * Look up the Func named by `key', or nullptr if it isn't currently loaded
 * or has changed since the key was made. Confirms the match by rebuilding
 * the key from the candidate.
 */
Func* resolveContProfFunc(const ContProfFuncKey&, Unit& resolutionUnit);

}
