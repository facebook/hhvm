/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <compression/zstd.h>
#include <my_byteorder.h>  // TODO: fix this include
#include <algorithm>
#include "wrapper_functions.h"

namespace binary_log {
namespace transaction {
namespace compression {

/*
   ****************************************************************
   Compressor
   ****************************************************************
 */

Zstd_comp::Zstd_comp()
    : m_ctx(nullptr),
      m_compression_level_current(DEFAULT_COMPRESSION_LEVEL),
      m_compression_level_next(DEFAULT_COMPRESSION_LEVEL) {
  // create the stream context
  if (m_ctx == nullptr) m_ctx = ZSTD_createCStream();

  // initialize the stream
  if (m_ctx != nullptr) {
    if (ZSTD_isError(ZSTD_initCStream(m_ctx, m_compression_level_current))) {
      /* purecov: begin inspected */
      // Abnormal error when initializing the context
      ZSTD_freeCStream(m_ctx);
      m_ctx = nullptr;
      /* purecov: end */
    }
  }
}

void Zstd_comp::set_compression_level(unsigned int clevel) {
  if (clevel != m_compression_level_current) {
    m_compression_level_next = clevel;
  }
}

Zstd_comp::~Zstd_comp() {
  if (m_ctx != nullptr) {
    ZSTD_freeCStream(m_ctx);
    m_ctx = nullptr;
  }

  m_buffer_cursor = m_buffer;
}

type Zstd_comp::compression_type_code() { return ZSTD; }

bool Zstd_comp::open() {
  size_t ret{0};
  if (m_ctx == nullptr) goto err;

    /*
     The advanced compression API used below as declared stable in
     1.4.0 .

     The advanced API allows reusing the same context instead of
     creating a new one every time we open the compressor. This
     is useful within the binary log compression.
    */
#if ZSTD_VERSION_NUMBER >= 10400
  if (m_compression_level_current == m_compression_level_next) {
    ret = ZSTD_CCtx_reset(m_ctx, ZSTD_reset_session_only);
    if (ZSTD_isError(ret)) goto err;

    ret = ZSTD_CCtx_setPledgedSrcSize(m_ctx, ZSTD_CONTENTSIZE_UNKNOWN);
    if (ZSTD_isError(ret)) goto err;
  } else {
    ret = ZSTD_initCStream(m_ctx, m_compression_level_next);
    if (ZSTD_isError(ret)) goto err;
    m_compression_level_current = m_compression_level_next;
  }
#else
  ret = ZSTD_initCStream(m_ctx, m_compression_level_next);
  if (ZSTD_isError(ret)) goto err;
  m_compression_level_current = m_compression_level_next;
#endif

  m_buffer_cursor = m_buffer;

  return false;
err:
  return true;
}

bool Zstd_comp::close() {
  size_t ret{0};
  if (m_ctx == nullptr) goto err;

  do {
    ret = ZSTD_flushStream(m_ctx, &m_obuf);
    if (ZSTD_isError(ret)) goto err;
    m_buffer_cursor = static_cast<unsigned char *>(m_obuf.dst) + m_obuf.pos;

    // end the stream
    ret = ZSTD_endStream(m_ctx, &m_obuf);
    if (ZSTD_isError(ret)) goto err;
    m_buffer_cursor = static_cast<unsigned char *>(m_obuf.dst) + m_obuf.pos;
  } while (ret > 0);

  return false;
err:
  return true;
}

std::tuple<std::size_t, bool> Zstd_comp::compress(const unsigned char *buffer,
                                                  size_t length) {
  m_obuf = {m_buffer, capacity(), size()};
  ZSTD_inBuffer ibuf{static_cast<const void *>(buffer), length, 0};
  std::size_t ret{0};
  auto err{false};

  while (ibuf.pos < ibuf.size) {
    std::size_t min_capacity{ZSTD_CStreamOutSize()};

    // always have at least one block available
    if ((err = reserve(min_capacity))) break;

    // adjust the obuf
    m_obuf.dst = m_buffer;
    m_obuf.size = capacity();

    // compress now
#if ZSTD_VERSION_NUMBER >= 10400
    ret = ZSTD_compressStream2(m_ctx, &m_obuf, &ibuf, ZSTD_e_continue);
#else
    ret = ZSTD_compressStream(m_ctx, &m_obuf, &ibuf);
#endif

    // adjust the cursor
    m_buffer_cursor = m_buffer + m_obuf.pos;

    if ((err = ZSTD_isError(ret))) break;
  }

  return std::make_tuple(ibuf.size - ibuf.pos, err);
}

/*
   ****************************************************************
   Decompressor
   ****************************************************************
 */

Zstd_dec::Zstd_dec() : m_ctx(nullptr) {
  // create the stream context
  m_ctx = ZSTD_createDStream();
  if (m_ctx != nullptr) {
    size_t ret = ZSTD_initDStream(m_ctx);
    if (ZSTD_isError(ret)) {
      ZSTD_freeDStream(m_ctx);
      m_ctx = nullptr;
    }
  }
}

Zstd_dec::~Zstd_dec() {
  if (m_ctx != nullptr) ZSTD_freeDStream(m_ctx);
}

type Zstd_dec::compression_type_code() { return ZSTD; }

bool Zstd_dec::open() {
  size_t ret{0};
  if (m_ctx == nullptr) return true;
    /*
     The advanced compression API used below as declared stable in
     1.4.0.

     The advanced API allows reusing the same context instead of
     creating a new one every time we open the compressor. This
     is useful within the binary log compression.
    */
#if ZSTD_VERSION_NUMBER >= 10400
  /** Reset session only. Dictionary will remain. */
  ret = ZSTD_DCtx_reset(m_ctx, ZSTD_reset_session_only);
#else
  ret = ZSTD_initDStream(m_ctx);
#endif

  return ZSTD_isError(ret);
}

bool Zstd_dec::close() {
  if (m_ctx == nullptr) return true;
  return false;
}

std::tuple<std::size_t, bool> Zstd_dec::decompress(const unsigned char *in,
                                                   size_t in_size) {
  ZSTD_outBuffer obuf{m_buffer, capacity(), size()};
  ZSTD_inBuffer ibuf{in, in_size, 0};
  std::size_t ret{0};
  auto err{false};

  do {
    auto min_buffer_len{ZSTD_DStreamOutSize()};

    // make sure that we have buffer space to hold the results
    if ((err = reserve(min_buffer_len))) break;

    // update the obuf buffer pointer and offset
    obuf.dst = m_buffer;
    obuf.size = capacity();

    // decompress
    ret = ZSTD_decompressStream(m_ctx, &obuf, &ibuf);

    // update the cursor
    m_buffer_cursor = m_buffer + obuf.pos;

    // error handling
    if ((err = ZSTD_isError(ret))) break;

  } while (obuf.size == obuf.pos);

  return std::make_tuple((ibuf.size - ibuf.pos), err);
}

}  // namespace compression
}  // namespace transaction
}  // namespace binary_log
