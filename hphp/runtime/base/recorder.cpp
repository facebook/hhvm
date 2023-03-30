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

#include "hphp/runtime/base/recorder.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

#include <folly/Likely.h>
#include <folly/Random.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/util/assertions.h"
#include "hphp/util/blob-encoder.h"
#include "hphp/util/build-info.h"

namespace HPHP {

void Recorder::onFunctionReturn(const ActRec* ar, TypedValue ret) {
  if (UNLIKELY(m_enabled && ar->func()->nativeFuncPtr() != nullptr)) {
    auto& call{m_nativeCalls[ar->func()->fullNameStr().get()].emplace_back()};
    tvDup(ret, call.ret);
    DictInit args{ar->func()->numInOutParams()};
    for (std::size_t i{0}; i < ar->func()->numParams(); i++) {
      if (ar->func()->isInOut(i)) {
        args.set(i, *frame_local(ar, i));
      }
    }
    call.args = args.toArray();
  }
}

void Recorder::requestExit() {
  if (UNLIKELY(m_enabled)) {
    const auto dir{FileUtil::expandUser(RO::EvalRecordDir) + '/'};
    FileUtil::mkdir(dir);
    const auto file{dir + std::to_string(folly::Random::rand64()) + ".hhvm"};
    BlobEncoder encoder;
    toArray().serde(encoder);
    const auto data{encoder.take()};
    std::ofstream ofs{file, std::ios::binary};
    assertx(!ofs.fail());
    ofs.write(data.data(), data.size());
    m_enabled = false;
    m_nativeCalls.clear();
  }
}

void Recorder::requestInit() {
  if (UNLIKELY(m_enabled = folly::Random::oneIn64(RO::EvalRecordSampleRate))) {
    EventHook::Enable();
  }
}

void Recorder::setEntryPoint(const String& entryPoint) {
  m_entryPoint = entryPoint;
}

Array Recorder::toArray() const {
  DictInit files{g_context->m_evaledFiles.size()};
  for (const auto& [k, _] : g_context->m_evaledFiles) {
    const auto path{k->data()};
    std::ostringstream oss;
    oss << std::ifstream{path}.rdbuf();
    files.set(StringData::Make(path), oss.str());
  }
  DictInit nativeCalls{m_nativeCalls.size()};
  for (const auto& [name, calls] : m_nativeCalls) {
    VecInit nativeNameCalls{calls.size()};
    for (const auto& call : calls) {
      nativeNameCalls.append(make_vec_array(
        Variant::wrap(call.ret),
        call.args
      ));
    }
    nativeCalls.set(name->slice(), nativeNameCalls.toVariant());
  }
  return make_dict_array(
    "header", make_dict_array(
      "compilerId", compilerId(),
      "entryPoint", m_entryPoint
    ),
    "files", files.toArray(),
    "nativeCalls", nativeCalls.toArray()
  );
}

} // namespace HPHP
