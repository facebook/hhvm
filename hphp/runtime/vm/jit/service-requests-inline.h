/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_VM_SERVICE_REQUESTS_INLINE_H_
#define incl_HPHP_RUNTIME_VM_SERVICE_REQUESTS_INLINE_H_

#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

inline ServiceReqArgInfo ccServiceReqArgInfo(jit::ConditionCode cc) {
  return ServiceReqArgInfo{ServiceReqArgInfo::CondCode, { uint64_t(cc) }};
}

//////////////////////////////////////////////////////////////////////

struct SRArgMaker {
  ServiceReqArgVec& argv;

  template<class Head, class... Tail>
  void go(Head h, Tail... tail) {
    pack(h);
    go(tail...);
  }
  void go() {}

private:
  template<class T>
  typename std::enable_if<
    std::is_integral<T>::value ||
      std::is_pointer<T>::value ||
      std::is_enum<T>::value
  >::type pack(T arg) {
    // For things with a sensible cast to uint64_T, we assume we meant to pass
    // an immediate arg.
    argv.push_back({ ServiceReqArgInfo::Immediate, { uint64_t(arg) } });
  }

  void pack(const ServiceReqArgInfo& argInfo) {
    argv.push_back(argInfo);
  }
};

template<class... Args>
ServiceReqArgVec packServiceReqArgs(Args&&... args) {
  auto ret = ServiceReqArgVec{};
  SRArgMaker{ret}.go(std::forward<Args>(args)...);
  return ret;
}

//////////////////////////////////////////////////////////////////////

inline bool isEphemeralServiceReq(ServiceRequest sr) {
  return sr == REQ_BIND_JMPCC_FIRST ||
         sr == REQ_BIND_JMP ||
         sr == REQ_BIND_ADDR;
}

template<typename... Arg>
TCA emitServiceReq(CodeBlock& cb,
                   SRFlags flags,
                   folly::Optional<FPInvOffset> spOff,
                   ServiceRequest sr,
                   Arg... a) {
  // These should reuse stubs. Use emitEphemeralServiceReq.
  assertx(!isEphemeralServiceReq(sr));

  auto const argv = packServiceReqArgs(a...);
  return mcg->backEnd().emitServiceReqWork(
    cb,
    cb.frontier(),
    flags | SRFlags::Persist,
    spOff,
    sr,
    argv
  );
}

template<typename... Arg>
TCA emitEphemeralServiceReq(CodeBlock& cb,
                            TCA start,
                            folly::Optional<FPInvOffset> spOff,
                            ServiceRequest sr,
                            Arg... a) {
  assertx(isEphemeralServiceReq(sr) || sr == REQ_RETRANSLATE);
  return mcg->backEnd().emitServiceReqWork(
    cb,
    start,
    SRFlags::None,
    spOff,
    sr,
    packServiceReqArgs(a...)
  );
}

//////////////////////////////////////////////////////////////////////

}}

namespace std {

template<> struct hash<HPHP::jit::ServiceRequest> {
  size_t operator()(const HPHP::jit::ServiceRequest& sr) const {
    return sr;
  }
};

}

#endif
