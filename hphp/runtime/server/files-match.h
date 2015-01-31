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

#ifndef incl_HPHP_FILES_MATCH_H_
#define incl_HPHP_FILES_MATCH_H_

#include <string>
#include <vector>

#include "hphp/runtime/base/ini-setting.h"
#include "hphp/util/hdf.h"
#include "hphp/util/text-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class FilesMatch {
public:
  explicit FilesMatch(const IniSetting::Map& ini, const Hdf& vh);

  bool match(const std::string &filename) const;
  const std::vector<std::string> &getHeaders() const { return m_headers;}

private:
  std::string m_pattern;
  std::vector<std::string> m_headers;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_FILES_MATCH_H_
