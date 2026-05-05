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

template<typename ThrowUnexpectedNamedArgs,
         typename ThrowNamedArgMismatch, typename ThrowMissingNamedParam>
void checkNamedArgMismatch(const Func* callee, const ArrayData* namedArgNames,
                           ThrowUnexpectedNamedArgs throwUnexpectedNamedArgs,
                           ThrowNamedArgMismatch throwNamedArgMismatch,
                           ThrowMissingNamedParam throwMissingNamedParameter) {
  uint32_t namedParamCount = callee->numNamedParams();
  auto const namedArgNamesSize = namedArgNames == nullptr ? 0 : namedArgNames->size();
  if (namedParamCount == 0) {
    if (namedArgNamesSize > 0) {
      // This is a special case inserted to speed up the JIT - the JIT
      // short-circuits inspecting the named params at all if a function doesn't
      // have named params, so matching that behavior ensures the error messages
      // are consistent.
      throwUnexpectedNamedArgs(callee);
    }
    return;
  }
  auto const namedParamNames = callee->sortedNamedParamNames();

  auto paramIdx = 0;
  for (auto argIdx = 0; argIdx < namedArgNamesSize; ++argIdx) {
    auto argName = namedArgNames->at(argIdx).val().pstr;
    while (paramIdx < namedParamCount && namedParamNames[paramIdx] != argName) {
      if (!callee->params()[paramIdx].hasDefaultValue()) {
        throwMissingNamedParameter(callee, namedParamNames[paramIdx]);
        return;
      }
      ++paramIdx;
    }
    if (paramIdx >= namedParamCount) {
      throwNamedArgMismatch(callee, argName);
      return;
    }
    ++paramIdx;
  }
  while (paramIdx < namedParamCount) {
    if (!callee->params()[paramIdx].hasDefaultValue()) {
      throwMissingNamedParameter(callee, namedParamNames[paramIdx]);
      return;
    }
    ++paramIdx;
  }
}


/*
 * Given a stack top that points to the last passed argument for a callee,
 * re-arranges the stack to push numNamedParams - numNamedArgs uninits in
 * the right locations. If the callee expects reified generics, the stack
 * top is expected to point to the reified generics location.
 */
inline void initFuncNamedParamDefaultValues(const Func* callee,
                                            uint32_t posArgc,
                                            const ArrayData* namedArgNames,
                                            TypedValue* stackTop) {
  int32_t numNamedArgs = namedArgNames ? namedArgNames->size() : 0;
  bool hasVariadics = posArgc > callee->numPositionalParams();
  auto const numArgs = posArgc + numNamedArgs;
  assertx(callee->numNamedParams() >= numNamedArgs);
  if (callee->numNamedParams() == numNamedArgs) {
    return;
  }

  int32_t delta = callee->numNamedParams() - numNamedArgs;
  assertx(delta > 0);
  // We might have an extra variadics arg if we're in the
  // too-many-args prologue. hasVariadics will handle it properly.
  assertx(numArgs + delta <= callee->numParams() + 1);

  // This logic runs *after* the reified generics checks, meaning that for callees
  // with reified generics, the stack's top will be the reified generics type
  // structure. Move it to its final location.
  if (callee->hasReifiedGenerics()) {
    stackTop[-delta] = *stackTop;
    ++stackTop;
  }

  // We don't need to consider inouts here as they're mutually exclusive with named
  // params.
  // If there are no args present, this will point to the first unassigned location
  // after the stack top, which is the right place to push the first optional
  // named arg.
  auto firstArgLoc = stackTop + numArgs - 1;
  if (hasVariadics) {
    stackTop[-delta] = *stackTop;
  }

  PackedStringPtr const* namedParamNames = callee->sortedNamedParamNames();
  int32_t argIdx = numArgs - 1 - hasVariadics;
  for (int32_t paramIdx = posArgc + callee->numNamedParams() - 1 - hasVariadics; paramIdx >= 0; --paramIdx) {
    assertx(IMPLIES(paramIdx < callee->numNamedParams(), argIdx < numNamedArgs));
    if (paramIdx >= callee->numNamedParams() ||
        (argIdx >= 0 &&
         namedParamNames[paramIdx] == namedArgNames->at(argIdx).val().pstr)) {
      firstArgLoc[-paramIdx] = firstArgLoc[-argIdx];
      argIdx--;
    } else {
      auto const& DEBUG_ONLY param = callee->params()[paramIdx];
      assertx(param.hasDefaultValue());
      // Push uninits for any non-passed named params.
      firstArgLoc[-paramIdx] = make_tv<KindOfUninit>();
    }
  }
}

}
