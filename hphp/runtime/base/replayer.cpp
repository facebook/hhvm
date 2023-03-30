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

#include "hphp/runtime/base/replayer.h"

#include <cstddef>
#include <fstream>
#include <vector>

#include <folly/Likely.h>

#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/assertions.h"
#include "hphp/util/blob-encoder.h"
#include "hphp/util/build-info.h"

namespace HPHP {

String Replayer::file(String path) {
  return m_files[path].asCStrRef();
}

String Replayer::init(String path) {
  EventHook::Enable();
  std::ifstream ifs{path.data(), std::ios::binary | std::ios::ate};
  assertx(!ifs.fail());
  std::vector<char> data(static_cast<std::size_t>(ifs.tellg()));
  ifs.seekg(0).read(data.data(), data.size());
  BlobDecoder decoder{data.data(), data.size()};
  Array recording;
  recording.serde(decoder);
  m_files = recording[String{"files"}].asCArrRef();
  m_nativeCalls = recording[String{"nativeCalls"}].asCArrRef();
  const auto header{recording[String{"header"}].asCArrRef()};
  assertx(header[String{"compilerId"}].asCStrRef() == compilerId());
  return header[String{"entryPoint"}].asCStrRef();
}

bool Replayer::onFunctionCall(ActRec* ar) {
  if (UNLIKELY(ar->func()->nativeFuncPtr() != nullptr)) {
    const auto name{ar->func()->fullNameStr().asString()};
    assertx(m_nativeCalls.exists(name));
    const auto& calls{m_nativeCalls[name].asArrRef()};
    assertx(!calls.empty());
    Variant call;
    calls->popMove(call);
    for (std::int32_t i{0}; i < ar->func()->numParams(); i++) {
      if (ar->func()->isInOut(i)) {
        tvDup(call.asArrRef()[1].asArrRef()[i].detach(), vmStack().indTV(i));
      }
    }
    tvDup(call.asArrRef()[0].detach(), vmStack().allocTV());
    return false;
  }
  return true;
}

} // namespace HPHP
