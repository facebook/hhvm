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

#include "hphp/runtime/server/files-match.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/config.h"
#include "hphp/util/text-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FilesMatch::FilesMatch(const IniSetting::Map& ini, Hdf vh) {
  m_pattern = format_pattern(Config::Get(ini, vh["pattern"], ""), true);
  Config::Get(ini, vh["headers"], m_headers);
}

bool FilesMatch::match(const std::string &filename) const {
  if (!m_pattern.empty()) {
    Variant ret = preg_match(String(m_pattern.c_str(), m_pattern.size(),
                                    CopyString),
                             String(filename.c_str(), filename.size(),
                                    CopyString));
    return ret.toInt64() > 0;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
