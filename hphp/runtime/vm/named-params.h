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

}
