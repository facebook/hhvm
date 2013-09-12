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

#ifndef incl_HPHP_DIRECTORY_H_
#define incl_HPHP_DIRECTORY_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/array-iterator.h"

#include <dirent.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Directory : public SweepableResourceData {
public:
  virtual void close() = 0;
  virtual String read() = 0;
  virtual void rewind() = 0;
  void sweep() {
    close();
  }

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassNameHook() const { return s_class_name; }

  String getLastError() {
    return String(folly::errnoStr(errno).toStdString());
  }
};

class PlainDirectory : public Directory {
public:
  DECLARE_RESOURCE_ALLOCATION(PlainDirectory);

  explicit PlainDirectory(CStrRef path);
  ~PlainDirectory();

  virtual void close();
  virtual String read();
  virtual void rewind();
  bool isValid() const;

private:
  DIR *m_dir;
};

class ArrayDirectory : public Directory {
public:
  DECLARE_RESOURCE_ALLOCATION(ArrayDirectory);

  explicit ArrayDirectory(CArrRef a) : m_it(a) {}

  virtual void close() {}
  virtual String read();
  virtual void rewind();

private:
  ArrayIter m_it;

};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DIRECTORY_H_
