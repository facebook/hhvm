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

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

#include <folly/Optional.h>

#include <type_traits>

namespace HPHP { namespace jit { namespace svcreq {

///////////////////////////////////////////////////////////////////////////////

namespace detail {

///////////////////////////////////////////////////////////////////////////////

struct ArgPacker {
  ArgVec& argv;

  template<class Head, class... Tail>
  void go(Head h, Tail... tail) {
    pack(h);
    go(tail...);
  }
  void go() {}

private:
  void pack(const Arg& arg) { argv.emplace_back(arg); }
  void pack(TCA addr) { argv.emplace_back(addr); }
  void pack(ConditionCode cc) { argv.emplace_back(cc); }

  template<class T>
  typename std::enable_if<
    std::is_integral<T>::value ||
    std::is_pointer<T>::value ||
    std::is_enum<T>::value
  >::type pack(T arg) {
    // For anything else with a sensible cast to uint64_t, we assume we meant
    // to pass an immediate arg.
    argv.emplace_back(Arg(uint64_t(arg)));
  }
};

/*
 * Pack a variadic list of service request args into an ArgVec.
 *
 * Arguments that aren't any of the Arg types are interpreted as immediates
 * (i.e., uint64_t) if possible.
 */
template<class... Args>
ArgVec pack_args(Args&&... args) {
  auto ret = ArgVec{};
  ArgPacker{ret}.go(std::forward<Args>(args)...);
  return ret;
}

/*
 * Return true iff `sr' is a service request that is always ephemeral.
 */
inline bool is_ephemeral(ServiceRequest sr) {
  return sr == REQ_BIND_JMP ||
         sr == REQ_BIND_ADDR ||
         sr == REQ_BIND_JCC_FIRST;
}

/*
 * Service request stub emitter.
 *
 * Declared here for use in the templatized stub emitters defined below.
 */
void emit_svcreq(CodeBlock& cb, TCA start, bool persist,
                 folly::Optional<FPInvOffset> spOff,
                 ServiceRequest sr, const ArgVec& argv);

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

template<typename... Args>
TCA emit_persistent(CodeBlock& cb,
                    folly::Optional<FPInvOffset> spOff,
                    ServiceRequest sr,
                    Args... args) {
  using namespace detail;
  assertx(!is_ephemeral(sr));

  auto const start = cb.frontier();
  emit_svcreq(cb, cb.frontier(), true, spOff, sr, pack_args(args...));
  return start;
}

template<typename... Args>
TCA emit_ephemeral(CodeBlock& cb,
                   TCA start,
                   folly::Optional<FPInvOffset> spOff,
                   ServiceRequest sr,
                   Args... args) {
  using namespace detail;
  assertx(is_ephemeral(sr) || sr == REQ_RETRANSLATE);

  emit_svcreq(cb, start, false, spOff, sr, pack_args(args...));
  return start;
}

///////////////////////////////////////////////////////////////////////////////

}}}

namespace std {

///////////////////////////////////////////////////////////////////////////////

template<> struct hash<HPHP::jit::ServiceRequest> {
  size_t operator()(const HPHP::jit::ServiceRequest& sr) const {
    return sr;
  }
};

///////////////////////////////////////////////////////////////////////////////

}
