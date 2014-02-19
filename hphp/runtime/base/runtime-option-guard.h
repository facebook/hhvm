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

#ifndef incl_HPHP_RUNTIME_BASE_OPTION_GUARD
#define incl_HPHP_RUNTIME_BASE_OPTION_GUARD

#include <type_traits>

#include "folly/Preprocessor.h"

namespace HPHP {

/*
 * RAII helper to temporarily set a runtime option to a new value. Use with
 * OPTION_GUARD:
 *
 * {
 *   OPTION_GUARD(EvalHHIRRefcountOpts, 0);
 *   // do stuff
 * }
 */
template<typename T>
struct OptionGuard {
  OptionGuard(T& option, T newVal)
    : m_option(option)
    , m_oldValue(option)
  {
    m_option = newVal;
  }

  ~OptionGuard() {
    m_option = m_oldValue;
  }

private:
  T& m_option;
  T const m_oldValue;
};

#define OPTION_GUARD(name, newVal)                                      \
  OptionGuard<std::remove_reference<decltype(RuntimeOption::name)>::type> \
  FB_ANONYMOUS_VARIABLE(RUNTIME_OPT_GUARD)(RuntimeOption::name, (newVal))

}

#endif
