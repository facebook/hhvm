/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_KERNEL_VERSION_H_
#define incl_HPHP_KERNEL_VERSION_H_

namespace HPHP {

struct KernelVersion {
  // <major>.<dot>.<dotdot>-<dash>_fbk<fbk>
  int m_major;
  int m_dot;
  int m_dotdot;
  int m_dash;
  int m_fbk;
  KernelVersion();             // Use uname
  explicit KernelVersion(const char*);  // A known kernel version for cmp.
  static int cmp(const KernelVersion& l, const KernelVersion& r) {
#define C(field) if (l.field != r.field) return l.field - r.field;
    C(m_major);
    C(m_dot);
    C(m_dotdot);
    C(m_dash);
    C(m_fbk);
#undef C
    return 0;
  }
  private:
  void parse(const char* c);
};

}

#endif
