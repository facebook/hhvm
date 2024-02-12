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

#include "hphp/util/hdf-extract.h"

#include "hphp/util/hdf.h"

namespace HPHP {

void hdfExtract(const Hdf& hdf, const char* name, bool& val, bool dv) {
  val = hdf[name].configGetBool(dv);
}

void hdfExtract(const Hdf& hdf, const char* name, uint16_t& val, uint16_t dv) {
  val = hdf[name].configGetUInt16(dv);
}

void hdfExtract(
  const Hdf& hdf,
  const char* name,
  std::map<std::string, std::string>& map,
  const std::map<std::string, std::string>& dv
) {
  Hdf config = hdf[name];
  if (config.exists() && !config.isEmpty()) config.configGet(map);
  else map = dv;
}

void hdfExtract(
  const Hdf& hdf,
  const char* name,
  std::vector<std::string>& vec,
  const std::vector<std::string>& dv
) {
  Hdf config = hdf[name];
  if (config.exists() && !config.isEmpty()) config.configGet(vec);
  else vec = dv;
}

void hdfExtract(
  const Hdf& hdf,
  const char* name,
  std::string& val,
  std::string dv
) {
  val = hdf[name].configGetString(dv);
}

} // namespace HPHP
