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

#pragma once

#include "hphp/util/string-holder.h"

#include <memory>
#include <cstddef>

#include <brotli/encode.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
struct BrotliCompressor {
  BrotliCompressor(BrotliEncoderMode mode, uint32_t quality, uint32_t lgWin);
  StringHolder compress(const void* data,
                        size_t& len,
                        bool last);
 private:
  struct EncStateDeleter {
    void operator()(BrotliEncoderState* e) const {
      BrotliEncoderDestroyInstance(e);
    }
  };
  std::unique_ptr<BrotliEncoderState, EncStateDeleter> m_encState;
};

///////////////////////////////////////////////////////////////////////////////
}
