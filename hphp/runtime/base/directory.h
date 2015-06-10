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

#ifndef incl_HPHP_DIRECTORY_H_
#define incl_HPHP_DIRECTORY_H_

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"

#include <dirent.h>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Variant;

struct Directory : SweepableResourceData {
  virtual void close() = 0;
  virtual Variant read() = 0;
  virtual void rewind() = 0;
  virtual Array getMetaData();
  virtual bool isEof() const {
    return false; // Most implementations can't tell if they've reached EOF
  }
  void sweep() override { close(); }

  CLASSNAME_IS("Directory")
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  String getLastError() {
    return String(folly::errnoStr(errno).toStdString());
  }
};

struct PlainDirectory : Directory {
  DECLARE_RESOURCE_ALLOCATION(PlainDirectory);

  explicit PlainDirectory(const String& path);
  ~PlainDirectory();

  void close() override;
  Variant read() override;
  void rewind() override;
  bool isValid() const;

private:
  DIR* m_dir;
};

struct ArrayDirectory : Directory {
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(ArrayDirectory);

  explicit ArrayDirectory(const Array& a) : m_it(a) {}

  void close() override {}
  Variant read() override;
  void rewind() override;
  bool isEof() const override;

  void sweep() override {
    // Leave m_it alone
    Directory::sweep();
  }

  size_t size() const { return m_it.getArrayData()->size(); }
  String path();

private:
  ArrayIter m_it;
};

struct CachedDirectory : Directory {
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(CachedDirectory);

  explicit CachedDirectory(const String& path);

  void close() override {}
  Variant read() override {
    if (m_pos >= m_files.size()) return false;
    return String(m_files[m_pos++]);
  }
  void rewind() override { m_pos = 0; }
  bool isEof() const override { return m_pos >= m_files.size(); }

private:
  int m_pos {0};
  std::vector<std::string> m_files;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DIRECTORY_H_
