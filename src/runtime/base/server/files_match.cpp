/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/complex_types.h>
#include <runtime/base/server/files_match.h>
#include <runtime/base/server/virtual_host.h>
#include <runtime/base/preg.h>
#include <util/util.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FilesMatch::FilesMatch(Hdf vh) {
  m_pattern = Util::format_pattern(vh["pattern"].get(""), true);
  vh["headers"].get(m_headers);
}

bool FilesMatch::match(const std::string &filename) const {
  if (!m_pattern.empty()) {
    Variant ret = preg_match(String(m_pattern.c_str(), m_pattern.size(),
                                    AttachLiteral),
                             String(filename.c_str(), filename.size(),
                                    AttachLiteral));
    return ret.toInt64() > 0;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
