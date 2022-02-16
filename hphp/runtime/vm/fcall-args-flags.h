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

namespace HPHP {

enum FCallArgsFlags : uint16_t {
  FCANone                      = 0,
  // Unpack remaining arguments from a varray passed by ...$args.
  HasUnpack                    = (1 << 0),
  // Pass generics to the callee.
  HasGenerics                  = (1 << 1),
  // Lock newly constructed object if unwinding the constructor call.
  LockWhileUnwinding           = (1 << 2),
  // Arguments are known to be compatible with prologue of the callee and
  // do not need to be repacked.
  SkipRepack                   = (1 << 3),
  // Coeffects are known to be correct, no need to check.
  // If the callee has polymoprhic coeffects, pass caller's coeffects.
  SkipCoeffectsCheck           = (1 << 4),
  // Indicates that the caller requires the return value to be mutable
  // (not readonly)
  EnforceMutableReturn         = (1 << 5),
  // Indicates that the callee should not modify its instance
  EnforceReadonlyThis          = (1 << 6),
  // HHBC-only: Op should be resolved using an explicit context class
  ExplicitContext              = (1 << 7),
  // HHBC-only: is the number of returns provided? false => 1
  HasInOut                     = (1 << 8),
  // HHBC-only: should this FCall enforce argument inout-ness?
  EnforceInOut                 = (1 << 9),
  // HHBC-only: should this FCall enforce argument readonly-ness?
  EnforceReadonly              = (1 << 10),
  // HHBC-only: is the async eager offset provided? false => kInvalidOffset
  HasAsyncEagerOffset          = (1 << 11),
  // HHBC-only: the remaining space is used for number of arguments
  NumArgsStart                 = (1 << 12),
};

constexpr FCallArgsFlags operator|(FCallArgsFlags a, FCallArgsFlags b) {
  return FCallArgsFlags((uint16_t)a | (uint16_t)b);
}
}
