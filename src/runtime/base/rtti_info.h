/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_RTTI_INFO_H__
#define __HPHP_RTTI_INFO_H__

#include <string>
#include <vector>
#include <util/mutex.h>
#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef unsigned int RTTICounter[MaxNumDataTypes];

class RTTIInfo {
public:
  static RTTIInfo TheRTTIInfo;
  void translate_rtti(const char *rttiDir);
  void loadMetaData(const char *filename);
  bool loadProfData(const char *rttiDir);
  bool exists(const char *funcName);

public:
  RTTIInfo();
  ~RTTIInfo() { if (m_profData) free(m_profData);}

  void init(bool createDir);
  int getCount() { return m_count;}

private:
  Mutex m_mutex;
  bool m_loaded;
  int m_count;
  std::vector<std::string> m_id2name;
  std::set<std::string> m_functions;
  RTTICounter *m_profData;

  void loadParamMap(const char **p);
};

unsigned int *getRTTICounter(int id);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_RTTI_INFO_H__
