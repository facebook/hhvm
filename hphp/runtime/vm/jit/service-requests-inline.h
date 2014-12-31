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

namespace HPHP { namespace jit {

inline ServiceReqArgInfo ccServiceReqArgInfo(jit::ConditionCode cc) {
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

inline bool isEphemeralServiceReq(ServiceRequest sr) {
  return sr == REQ_BIND_JMPCC_FIRST ||
         sr == REQ_BIND_JMPCC_SECOND ||
         sr == REQ_BIND_JMP ||
         sr == REQ_BIND_JCC ||
         sr == REQ_BIND_SIDE_EXIT ||
         sr == REQ_BIND_ADDR;
}

template<typename... Arg>
TCA emitServiceReq(CodeBlock& cb, SRFlags flags, ServiceRequest sr, Arg... a) {
  // These should reuse stubs. Use emitEphemeralServiceReq.
  assert(!isEphemeralServiceReq(sr));

  ServiceReqArgVec argv;
  packServiceReqArgs(argv, a...);
  return mcg->backEnd().emitServiceReqWork(cb, cb.frontier(),
                                           flags | SRFlags::Persist,
                                           sr, argv);
}

template<typename... Arg>
TCA emitServiceReq(CodeBlock& cb, ServiceRequest sr, Arg... a) {
  return emitServiceReq(cb, SRFlags::None, sr, a...);
}

template<typename... Arg>
TCA emitEphemeralServiceReq(CodeBlock& cb, TCA start, ServiceRequest sr,
                            Arg... a) {
  assert(isEphemeralServiceReq(sr) ||
         sr == REQ_RETRANSLATE);

  ServiceReqArgVec argv;
  packServiceReqArgs(argv, a...);
  return mcg->backEnd().emitServiceReqWork(cb, start, SRFlags::None, sr,
                                           argv);
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
