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

#include <map>
#include <string>
#include <vector>

namespace HPHP {

struct Hdf;

void hdfExtract(const Hdf& hdf, const char* name, bool& val, bool dv);

void hdfExtract(const Hdf& hdf, const char* name, uint16_t& val, uint16_t dv);

void hdfExtract(const Hdf& hdf, const char* name, std::map<std::string, std::string>& map,
                const std::map<std::string, std::string>& dv);

void hdfExtract(
  const Hdf& hdf,
  const char* name,
  std::vector<std::string>& vec,
  const std::vector<std::string>& dv
);

void hdfExtract(
  const Hdf& hdf,
  const char* name,
  std::string& val,
  std::string dv
);

} // namespace HPHP
