/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __CRUTCH_H__
#define __CRUTCH_H__

#include <runtime/base/base_includes.h>
#include <sys/types.h>
#include <unistd.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Crutch {
 public:
  static bool Enabled;
  static Array Invoke(String func, Array schema, Array params);

 private:
  static const int MSG_MAX_SIZE = 10 * 1024 * 1024;
  static Crutch s_singleton;
  static Mutex s_mutex;

  pid_t m_php;
  Object m_queue;

  Crutch();
  ~Crutch();
  void init();
  void terminate();
  Array invoke(String func, Array schema, Array params);
};

class OpaqueObject : public ResourceData {
 public:
  static Object GetObject(int index);
  static int GetIndex(Object obj);

  // overriding ResourceData
  const char *o_getClassName() const { return "OpaqueObject";}

 private:
  OpaqueObject(int index);
  int m_index; // array index of $Objects in crutch.php
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CRUTCH_H__
