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
#ifndef HPHP_PHP_STREAM_WRAPPER_H
#define HPHP_PHP_STREAM_WRAPPER_H

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/stream-wrapper.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct PhpStreamWrapper final : Stream::Wrapper {
  req::ptr<File> openFD(const char *sFD);
  req::ptr<File> open(const String& filename, const String& mode, int options,
                      const req::ptr<StreamContext>& context) override;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_PHP_STREAM_WRAPPER_H
