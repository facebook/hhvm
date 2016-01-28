/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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


#ifndef incl_HPHP_MEMORYSTATS_H_
#define incl_HPHP_MEMORYSTATS_H_

#include <iostream>
#include <mutex>
#include <memory>

#include "hphp/runtime/server/writer.h"

namespace HPHP {

class MemoryStats {
  struct StatM {
    size_t m_vmSize;
    size_t m_vmRss;
    size_t m_share;
    size_t m_text;
    size_t m_data;
  };

  public:
    void ReportMemory(std::string &out, Writer::Format format);
    static MemoryStats* GetInstance();
    void ResetStaticStringSize();
    void LogStaticStringAlloc(size_t bytes);
    MemoryStats() {}

  private:
    size_t GetStaticStringSize();
    bool FillProcessStatM(StatM* pStatM);
    std::atomic<size_t> m_staticStringSize;
};
}

#endif //incl_HPHP_MEMORYSTATS_H_
