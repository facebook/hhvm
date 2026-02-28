/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/util/health-monitor-types.h"
#include "hphp/util/text-util.h"

namespace HPHP {

struct ApacheExtension final : Extension {
  ApacheExtension();
  ~ApacheExtension() override;
  void moduleRegisterNative() override;

  static void UpdateHealthLevel(HealthLevel newStatus) {
    m_healthLevel = newStatus;
  }

  static HealthLevel GetHealthLevel() {
    return m_healthLevel;
  }

 private:
  static HealthLevel m_healthLevel;
};

static Array get_headers(const HeaderMap& headers, bool allHeaders = false) {
  DictInit ret(headers.size());
  for (auto& iter : headers) {
    const auto& values = iter.second;
    if (auto size = values.size()) {
      if (!allHeaders) {
        ret.set(String(iter.first), String(values.back()));
      } else {
        VecInit dups(size);
        for (auto& dup : values) {
          dups.append(String(dup));
        }
        ret.set(String(toLower(iter.first)), dups.toArray());
      }
    }
  }
  return ret.toArray();
}

}
