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
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP { namespace JIT {

inline ServiceReqArgInfo ccServiceReqArgInfo(JIT::ConditionCode cc) {
  return ServiceReqArgInfo{ServiceReqArgInfo::CondCode, { uint64_t(cc) }};
}

template<typename T>
typename std::enable_if<
  // Only allow for things with a sensible cast to uint64_t.
  std::is_integral<T>::value || std::is_pointer<T>::value ||
  std::is_enum<T>::value
  >::type packServiceReqArg(ServiceReqArgVec& args, T arg) {
  // By default, assume we meant to pass an immediate arg.
  args.push_back({ ServiceReqArgInfo::Immediate, { uint64_t(arg) } });
}

inline void packServiceReqArg(ServiceReqArgVec& args,
                       const ServiceReqArgInfo& argInfo) {
  args.push_back(argInfo);
}

template<typename T, typename... Arg>
void packServiceReqArgs(ServiceReqArgVec& argv, T arg, Arg... args) {
  packServiceReqArg(argv, arg);
  packServiceReqArgs(argv, args...);
}

inline void packServiceReqArgs(ServiceReqArgVec& argv) {
  // Recursive base case.
}

//////////////////////////////////////////////////////////////////////

template<typename... Arg>
TCA emitServiceReq(CodeBlock& cb, SRFlags flags, ServiceRequest sr, Arg... a) {
  // These should reuse stubs. Use emitEphemeralServiceReq.
  assert(sr != REQ_BIND_JMPCC_FIRST &&
         sr != REQ_BIND_JMPCC_SECOND &&
         sr != REQ_BIND_JMP);

  ServiceReqArgVec argv;
  packServiceReqArgs(argv, a...);
  return mcg->backEnd().emitServiceReqWork(cb, cb.frontier(), true, flags, sr,
                                           argv);
}

template<typename... Arg>
TCA emitServiceReq(CodeBlock& cb, ServiceRequest sr, Arg... a) {
  return emitServiceReq(cb, SRFlags::None, sr, a...);
}

template<typename... Arg>
TCA emitEphemeralServiceReq(CodeBlock& cb, TCA start, ServiceRequest sr,
                            Arg... a) {
  assert(sr == REQ_BIND_JMPCC_FIRST ||
         sr == REQ_BIND_JMPCC_SECOND ||
         sr == REQ_BIND_JMP);
  assert(cb.contains(start));

  ServiceReqArgVec argv;
  packServiceReqArgs(argv, a...);
  return mcg->backEnd().emitServiceReqWork(cb, start, false, SRFlags::None, sr,
                                           argv);
}

//////////////////////////////////////////////////////////////////////

}}

namespace std {

template<> struct hash<HPHP::JIT::ServiceRequest> {
  size_t operator()(const HPHP::JIT::ServiceRequest& sr) const {
    return sr;
  }
};

}

#endif
