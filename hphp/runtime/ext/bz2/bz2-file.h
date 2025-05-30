/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/util/type-scan.h"
#include <stdio.h>
#include <bzlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// BZ2File class

struct BZ2File : File {
  DECLARE_RESOURCE_ALLOCATION(BZ2File)

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  BZ2File();
  explicit BZ2File(req::ptr<PlainFile>&& innerFile);
  virtual ~BZ2File();

  bool open(const String& filename, const String& mode) override;
  bool close(int* unused = nullptr) final;
  int64_t errnu();
  String errstr();
  Array error();
  bool flush() override;
  int64_t readImpl(char * buf, int64_t length) override;
  int64_t writeImpl(const char * buf, int64_t length) override;
  bool eof() override;

private:
  BZFILE * m_bzFile;
  // BZFILE is a typedef to void.
  TYPE_SCAN_IGNORE_FIELD(m_bzFile);
  req::ptr<PlainFile> m_innerFile;
};

///////////////////////////////////////////////////////////////////////////////
}

