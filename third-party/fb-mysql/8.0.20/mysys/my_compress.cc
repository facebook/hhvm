/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_compress.cc
*/
#include "my_compress.h"

#include <lz4frame.h>
#include <string.h>
#include <sys/types.h>
#include <zlib.h>
#include <zstd.h>
#include <algorithm>
#include <cstddef>

#include <mysql_com.h>

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"

#ifdef MYSQL_SERVER
extern uint net_compression_level;
extern long zstd_net_compression_level;
extern long lz4f_net_compression_level;
#else
#define net_compression_level 6
#define zstd_net_compression_level 0
#define lz4f_net_compression_level 0
#endif

/* Number of times compression context was reset for streaming compression. */
ulonglong compress_ctx_reset = 0;
/* Number of in/out bytes for compression. */
ulonglong compress_input_bytes = 0;
ulonglong compress_output_bytes = 0;

void reset_compress_status(void) {
  compress_ctx_reset = 0;
  compress_input_bytes = 0;
  compress_output_bytes = 0;
}

/**
  Initialize a compress context object to be associated with a NET object.

  @param cmp_ctx Pointer to compression context.
  @param algorithm Compression algorithm.
  @param compression_level Compression level corresponding to the compression
  algorithm.
*/

void mysql_compress_context_init(mysql_compress_context *cmp_ctx,
                                 enum enum_compression_algorithm algorithm,
                                 unsigned int compression_level) {
  cmp_ctx->algorithm = algorithm;
  switch (algorithm) {
    case enum_compression_algorithm::MYSQL_ZLIB:
      cmp_ctx->u.zlib_ctx.compression_level = compression_level;
      break;

    case enum_compression_algorithm::MYSQL_ZSTD:
    case enum_compression_algorithm::MYSQL_ZSTD_STREAM:
      cmp_ctx->u.zstd_ctx.compression_level = compression_level;
      // This is set after connect phase during first network i/o.
      cmp_ctx->u.zstd_ctx.cctx = nullptr;
      cmp_ctx->u.zstd_ctx.dctx = nullptr;
      cmp_ctx->u.zstd_ctx.compress_buf = nullptr;
      cmp_ctx->u.zstd_ctx.compress_buf_len = 0;
      break;

    case enum_compression_algorithm::MYSQL_LZ4F_STREAM:
      cmp_ctx->u.lz4f_ctx.compression_level = compression_level;
      cmp_ctx->u.lz4f_ctx.cctx = nullptr;
      cmp_ctx->u.lz4f_ctx.dctx = nullptr;
      cmp_ctx->u.lz4f_ctx.compress_buf = nullptr;
      cmp_ctx->u.lz4f_ctx.compress_buf_len = 0;

    default:
      break;
  }
}

/**
  Deinitialize the compression context allocated.

  @param cmp_ctx Pointer to Compression context.
*/

void mysql_compress_context_deinit(mysql_compress_context *cmp_ctx) {
  switch (cmp_ctx->algorithm) {
    case enum_compression_algorithm::MYSQL_ZSTD:
    case enum_compression_algorithm::MYSQL_ZSTD_STREAM:
      if (cmp_ctx->u.zstd_ctx.cctx != nullptr) {
        ZSTD_freeCCtx(cmp_ctx->u.zstd_ctx.cctx);
        cmp_ctx->u.zstd_ctx.cctx = nullptr;
      }

      if (cmp_ctx->u.zstd_ctx.dctx != nullptr) {
        ZSTD_freeDCtx(cmp_ctx->u.zstd_ctx.dctx);
        cmp_ctx->u.zstd_ctx.dctx = nullptr;
      }
      cmp_ctx->u.zstd_ctx.compress_buf_len = 0;
      if (cmp_ctx->u.zstd_ctx.compress_buf) {
        my_free(cmp_ctx->u.zstd_ctx.compress_buf);
        cmp_ctx->u.zstd_ctx.compress_buf = nullptr;
      }
      break;

    case enum_compression_algorithm::MYSQL_LZ4F_STREAM:
      if (cmp_ctx->u.lz4f_ctx.cctx != nullptr) {
        LZ4F_freeCompressionContext(cmp_ctx->u.lz4f_ctx.cctx);
        cmp_ctx->u.lz4f_ctx.cctx = nullptr;
      }

      if (cmp_ctx->u.lz4f_ctx.dctx != nullptr) {
        LZ4F_freeDecompressionContext(cmp_ctx->u.lz4f_ctx.dctx);
        cmp_ctx->u.lz4f_ctx.dctx = nullptr;
      }
      cmp_ctx->u.lz4f_ctx.compress_buf_len = 0;
      if (cmp_ctx->u.lz4f_ctx.compress_buf) {
        my_free(cmp_ctx->u.lz4f_ctx.compress_buf);
        cmp_ctx->u.lz4f_ctx.compress_buf = nullptr;
      }
      break;

    default:
      break;
  }
}

/**
  Allocate zstd compression contexts if necessary and compress using zstd the
  buffer.

  @param comp_ctx Compression context info relating to zstd.
  @param packet   Data to compress. This is is replaced with the compressed
  data.
  @param len      Length of data to compress at 'packet'
  @param complen  out: 0 if packet was not compressed

  @return nullptr if error (len is not changed) else pointer to buffer.
  size of compressed packet).
*/

uchar *zstd_compress_alloc(mysql_zstd_compress_context *comp_ctx,
                           const uchar *packet, size_t *len, size_t *complen,
                           int compression_level) {
  if (comp_ctx->cctx == nullptr) {
    if (!(comp_ctx->cctx = ZSTD_createCCtx())) {
      return nullptr;
    }
  }

  size_t zstd_len = ZSTD_compressBound(*len);
  void *compbuf;
  size_t zstd_res;

  if (!(compbuf = my_malloc(PSI_NOT_INSTRUMENTED, zstd_len, MYF(MY_WME)))) {
    return nullptr;
  }

  zstd_res = ZSTD_compressCCtx(
      comp_ctx->cctx, compbuf, zstd_len, (const void *)packet, *len,
      compression_level ? compression_level : comp_ctx->compression_level);
  if (ZSTD_isError(zstd_res)) {
    DBUG_PRINT("error", ("Can't compress zstd packet, error: %zd, %s", zstd_res,
                         ZSTD_getErrorName(zstd_res)));
    my_free(compbuf);
    return nullptr;
  }

  if (zstd_res > *len) {
    *complen = 0;
    my_free(compbuf);
    DBUG_PRINT("note",
               ("Packet got longer on zstd compression; Not compressed"));
    return nullptr;
  }

  *complen = *len;
  *len = zstd_res;
  return (uchar *)compbuf;
}

/**
  Uncompress a zstd compressed data.

  @param comp_ctx         Pointer to compression context.
  @param packet           Packet with zstd compressed data.
  @param len              Length of zstd compressed packet.
  @param complen [out]    Length of uncompressed packet.

  @return true on error else false.
*/

static bool zstd_uncompress(mysql_zstd_compress_context *comp_ctx,
                            uchar *packet, size_t len, size_t *complen) {
  DBUG_ASSERT(comp_ctx != nullptr);
  size_t zstd_res;
  void *compbuf;

  if (comp_ctx->dctx == nullptr) {
    if (!(comp_ctx->dctx = ZSTD_createDCtx())) {
      return true;
    }
  }

  if (!(compbuf = my_malloc(PSI_NOT_INSTRUMENTED, *complen, MYF(MY_WME)))) {
    return true;
  }

  zstd_res = ZSTD_decompressDCtx(comp_ctx->dctx, compbuf, *complen,
                                 (const void *)packet, len);

  if (ZSTD_isError(zstd_res) || zstd_res != *complen) {
    DBUG_PRINT("error", ("Can't uncompress zstd packet, error: %zd, %s",
                         zstd_res, ZSTD_getErrorName(zstd_res)));
    my_free(compbuf);
    return true;
  }

  memcpy(packet, compbuf, *complen);
  my_free(compbuf);
  return false;
}

uchar *zstd_stream_compress_alloc(mysql_zstd_compress_context *zstd_ctx,
                                  const uchar *packet, size_t *len,
                                  size_t *complen, int compression_level) {
  size_t zstd_len = ZSTD_compressBound(*len);
  uchar *compbuf;
  size_t zstd_res;

  if (zstd_ctx->compress_buf_len < zstd_len) {
    my_free(zstd_ctx->compress_buf);
    if (!(compbuf = (uchar *)my_malloc(key_memory_my_compress_alloc, zstd_len,
                                       MYF(MY_WME)))) {
      return NULL;
    }
    zstd_ctx->compress_buf = compbuf;
    zstd_ctx->compress_buf_len = zstd_len;
  } else {
    compbuf = zstd_ctx->compress_buf;
  }

  ZSTD_inBuffer inBuf = {packet, *len, 0};
  ZSTD_outBuffer outBuf = {compbuf, zstd_len, 0};

  if (!zstd_ctx->cctx) {
    if (!(zstd_ctx->cctx = ZSTD_createCStream())) {
      return NULL;
    }
    zstd_res = ZSTD_initCStream(
        zstd_ctx->cctx,
        compression_level ? compression_level : zstd_ctx->compression_level);
    if (ZSTD_isError(zstd_res)) {
      goto error;
    }
  }

  zstd_res = ZSTD_compressStream(zstd_ctx->cctx, &outBuf, &inBuf);
  if (ZSTD_isError(zstd_res)) {
    DBUG_PRINT("error", ("Can't compress zstd_stream packet, error %zd, %s",
                         zstd_res, ZSTD_getErrorName(zstd_res)));
    goto error;
  }

  if (inBuf.pos != inBuf.size) {
    goto error;
  }

  zstd_res = ZSTD_flushStream(zstd_ctx->cctx, &outBuf);
  if (zstd_res != 0) {
    DBUG_PRINT("error", ("Can't flush zstd_stream packet, error %zd, %s",
                         zstd_res, ZSTD_getErrorName(zstd_res)));
    goto error;
  }

  if (outBuf.pos > *len) {
    DBUG_PRINT("note", ("Packet got longer on zstd_stream compression; Not "
                        "compressed, %zu -> %zu",
                        *len, outBuf.pos));
    goto nocompress;
  }

  *complen = *len;
  *len = outBuf.pos;

  return compbuf;

nocompress:
  *complen = 0;
error:
  ZSTD_CCtx_reset(zstd_ctx->cctx, ZSTD_reset_session_only);
  compress_ctx_reset++;
  return NULL;
}

bool zstd_stream_uncompress(mysql_zstd_compress_context *zstd_ctx,
                            uchar *packet, size_t len, size_t *complen) {
  unsigned long long decom_size = *complen;
  size_t zstd_res;
  void *decom_buf;
  if (!zstd_ctx->dctx) {
    if (!(zstd_ctx->dctx = ZSTD_createDStream())) {
      return true;
    }
    zstd_res = ZSTD_initDStream(zstd_ctx->dctx);
    if (ZSTD_isError(zstd_res)) {
      return true;
    }
  }

  DBUG_PRINT("note", ("zstd_stream uncompress %zu -> %zu", len, *complen));

  if (!(decom_buf =
            my_malloc(key_memory_my_compress_alloc, decom_size, MYF(MY_WME)))) {
    return true;
  }

  ZSTD_inBuffer inBuf = {packet, len, 0};
  ZSTD_outBuffer outBuf = {decom_buf, decom_size, 0};

  zstd_res = ZSTD_decompressStream(zstd_ctx->dctx, &outBuf, &inBuf);
  if (ZSTD_isError(zstd_res)) {
    DBUG_PRINT("error", ("Can't uncompress zstd_stream packet, error: %zd, %s",
                         zstd_res, ZSTD_getErrorName(zstd_res)));
    my_free(decom_buf);
    return true;
  }
  DBUG_ASSERT(outBuf.pos == outBuf.size);

  if (outBuf.pos != outBuf.size) {
    my_free(decom_buf);
    return true;
  }

  memcpy(packet, decom_buf, outBuf.pos);
  my_free(decom_buf);
  *complen = outBuf.pos;

  return false;
}

uchar *lz4f_stream_compress_alloc(mysql_lz4f_compress_context *lz4f_ctx,
                                  const uchar *packet, size_t *len,
                                  size_t *complen, int compression_level) {
  size_t lz4f_len = LZ4F_compressBound(*len, NULL) + LZ4F_HEADER_SIZE_MAX;
  uchar *compbuf;
  size_t lz4f_res;
  size_t pos = 0;

  if (lz4f_ctx->compress_buf_len < lz4f_len) {
    my_free(lz4f_ctx->compress_buf);
    if (!(compbuf = (uchar *)my_malloc(key_memory_my_compress_alloc, lz4f_len,
                                       MYF(MY_WME)))) {
      return NULL;
    }
    lz4f_ctx->compress_buf = compbuf;
    lz4f_ctx->compress_buf_len = lz4f_len;
  } else {
    compbuf = lz4f_ctx->compress_buf;
  }

  if (!lz4f_ctx->cctx) {
    lz4f_res = LZ4F_createCompressionContext(
        (LZ4F_compressionContext_t *)&lz4f_ctx->cctx, LZ4F_VERSION);
    if (LZ4F_isError(lz4f_res)) {
      DBUG_PRINT("error", ("Can't create lz4f_stream context, error %zd, %s",
                           lz4f_res, LZ4F_getErrorName(lz4f_res)));
      goto error;
    }
    if (!lz4f_ctx->cctx) {
      goto error;
    }
    lz4f_ctx->reset_cctx = true;
  }

  if (lz4f_ctx->reset_cctx) {
    DBUG_PRINT("note", ("lz4f_stream cctx reset"));
    LZ4F_preferences_t prefs;
    memset(&prefs, 0, sizeof(prefs));
    prefs.compressionLevel =
        compression_level ? compression_level : lz4f_ctx->compression_level;

    lz4f_res = LZ4F_compressBegin((LZ4F_compressionContext_t)lz4f_ctx->cctx,
                                  compbuf, lz4f_len, &prefs);
    if (LZ4F_isError(lz4f_res)) {
      DBUG_PRINT("error", ("Can't compress lz4f_stream packet, error %zd, %s",
                           lz4f_res, LZ4F_getErrorName(lz4f_res)));
      goto error;
    }

    lz4f_ctx->reset_cctx = false;
    pos += lz4f_res;
  }

  lz4f_res = LZ4F_compressUpdate(lz4f_ctx->cctx, compbuf + pos, lz4f_len - pos,
                                 packet, *len, NULL);
  if (LZ4F_isError(lz4f_res)) {
    DBUG_PRINT("error", ("Can't compress lz4f_stream packet, error %zd, %s",
                         lz4f_res, LZ4F_getErrorName(lz4f_res)));
    goto error;
  }
  pos += lz4f_res;

  lz4f_res = LZ4F_flush(lz4f_ctx->cctx, compbuf + pos, lz4f_len - pos, NULL);
  if (LZ4F_isError(lz4f_res)) {
    DBUG_PRINT("error", ("Can't flush lz4f_stream packet, error %zd, %s",
                         lz4f_res, LZ4F_getErrorName(lz4f_res)));
    goto error;
  }
  pos += lz4f_res;

  if (pos > *len) {
    DBUG_PRINT("note", ("Packet got longer on lz4f_stream compression; Not "
                        "compressed, %zu -> %zu",
                        *len, pos));
    goto nocompress;
  }

  *complen = *len;
  *len = pos;

  DBUG_PRINT("note", ("lz4f_stream compress %zu -> %zu", *complen, pos));
  return compbuf;

nocompress:
  *complen = 0;
error:
  compress_ctx_reset++;
  lz4f_ctx->reset_cctx = true;
  return NULL;
}

bool lz4f_stream_uncompress(mysql_lz4f_compress_context *lz4f_ctx,
                            uchar *packet, size_t len, size_t *complen) {
  size_t decom_size = *complen;
  size_t lz4f_res;
  uchar *decom_buf;

  DBUG_PRINT("note", ("lz4f_stream uncompress %zu -> %zu", len, *complen));
  if (!lz4f_ctx->dctx) {
    lz4f_res = LZ4F_createDecompressionContext(&lz4f_ctx->dctx, LZ4F_VERSION);
    if (LZ4F_isError(lz4f_res)) {
      DBUG_PRINT("error", ("Can't create lz4f_stream context, error %zd, %s",
                           lz4f_res, LZ4F_getErrorName(lz4f_res)));
      return true;
    }
    if (!lz4f_ctx->dctx) {
      return true;
    }
  }

  if (!(decom_buf = (uchar *)my_malloc(key_memory_my_compress_alloc, decom_size,
                                       MYF(MY_WME)))) {
    return true;
  }

  uchar *src_pos = packet;
  size_t src_len = len;
  uchar *dst_pos = decom_buf;
  size_t dst_len = decom_size;

  lz4f_res = LZ4F_decompress(lz4f_ctx->dctx, dst_pos, &dst_len, src_pos,
                             &src_len, NULL);
  if (LZ4F_isError(lz4f_res)) {
    DBUG_PRINT("error", ("Can't decompress lz4f_stream packet, error %zd, %s",
                         lz4f_res, LZ4F_getErrorName(lz4f_res)));
    my_free(decom_buf);
    return true;
  }

  dst_pos += dst_len;
  src_pos += src_len;

  // Assert that src and dst are consumed.
  DBUG_ASSERT(dst_pos == decom_buf + decom_size);
  DBUG_ASSERT(src_pos == packet + len);

  if (dst_pos != decom_buf + decom_size || src_pos != packet + len) {
    my_free(decom_buf);
    return true;
  }

  memcpy(packet, decom_buf, decom_size);
  my_free(decom_buf);
  *complen = decom_size;

  return false;
}

/**
  Allocate zlib compression contexts if necessary and compress using zlib the
  buffer.

  @param comp_ctx      Compression context info relating to zlib.
  @param packet        Data to compress. This is is replaced with the compressed
  data.
  @param len           Length of data to compress at 'packet'
  @param [out] complen 0 if packet was not compressed

  @return nullptr if error (len is not changed) else pointer to buffer.
  size of compressed packet).
*/

static uchar *zlib_compress_alloc(mysql_zlib_compress_context *comp_ctx,
                                  const uchar *packet, size_t *len,
                                  size_t *complen, uint compression_level) {
  uchar *compbuf;
  uLongf tmp_complen;
  int res;
  *complen = *len * 120 / 100 + 12;

  if (!(compbuf = (uchar *)my_malloc(key_memory_my_compress_alloc, *complen,
                                     MYF(MY_WME))))
    return nullptr; /* Not enough memory */

  tmp_complen = (uint)*complen;
  res = compress2(
      (Bytef *)compbuf, &tmp_complen, (Bytef *)const_cast<uchar *>(packet),
      (uLong)*len,
      compression_level ? compression_level : comp_ctx->compression_level);
  *complen = tmp_complen;

  if (res != Z_OK) {
    my_free(compbuf);
    return nullptr;
  }

  if (*complen >= *len) {
    *complen = 0;
    my_free(compbuf);
    DBUG_PRINT("note", ("Packet got longer on compression; Not compressed"));
    return nullptr;
  }
  /* Store length of compressed packet in *len */
  std::swap(*len, *complen);
  return compbuf;
}

/**
  Uncompress a zlib compressed data.

  @param packet           Packet which zstd compressed data.
  @param len              Length of zstd compressed packet.
  @param complen [out]    Length of uncompressed packet.

  @return true on error else false.
*/

static bool zlib_uncompress(uchar *packet, size_t len, size_t *complen) {
  uLongf tmp_complen;
  uchar *compbuf =
      (uchar *)my_malloc(key_memory_my_compress_alloc, *complen, MYF(MY_WME));
  int error;
  if (!compbuf) return true; /* Not enough memory */

  tmp_complen = (uint)*complen;
  error =
      uncompress((Bytef *)compbuf, &tmp_complen, (Bytef *)packet, (uLong)len);
  *complen = tmp_complen;
  if (error != Z_OK) { /* Probably wrong packet */
    DBUG_PRINT("error", ("Can't uncompress packet, error: %d", error));
    my_free(compbuf);
    return true;
  }
  memcpy(packet, compbuf, *complen);
  my_free(compbuf);
  return false;
}

/*
   This replaces the packet with a compressed packet

   SYNOPSIS
     my_compress()
     packet   Data to compress. This is is replaced with the compressed data.
     len      Length of data to compress at 'packet'
     complen  out: 0 if packet was not compressed

   RETURN
     1   error. 'len' is not changed'
     0   ok.  In this case 'len' contains the size of the compressed packet
*/

bool my_compress(mysql_compress_context *comp_ctx, uchar *packet, size_t *len,
                 size_t *complen) {
  DBUG_ENTER("my_compress");
  if (*len < MIN_COMPRESS_LENGTH &&
      comp_ctx->algorithm != enum_compression_algorithm::MYSQL_ZSTD_STREAM &&
      comp_ctx->algorithm != enum_compression_algorithm::MYSQL_LZ4F_STREAM) {
    *complen = 0;
    DBUG_PRINT("note", ("Packet too short: Not compressed"));
  } else {
    uchar *compbuf = my_compress_alloc(comp_ctx, packet, len, complen);
    if (!compbuf) {
      if (*complen == 0) {
        compress_output_bytes += *len;
        DBUG_RETURN(0);
      } else {
        DBUG_RETURN(1);
      }
    }
    memcpy(packet, compbuf, *len);

    switch (comp_ctx->algorithm) {
      case enum_compression_algorithm::MYSQL_ZSTD_STREAM:
        if (comp_ctx->u.zstd_ctx.compress_buf != compbuf) my_free(compbuf);
        break;
      case enum_compression_algorithm::MYSQL_LZ4F_STREAM:
        if (comp_ctx->u.lz4f_ctx.compress_buf != compbuf) my_free(compbuf);
        break;
      default:
        my_free(compbuf);
        break;
    }
  }
  compress_output_bytes += *len;
  DBUG_RETURN(0);
}

uchar *my_compress_alloc(mysql_compress_context *comp_ctx, const uchar *packet,
                         size_t *len, size_t *complen) {
  if (comp_ctx->algorithm == enum_compression_algorithm::MYSQL_LZ4F_STREAM)
    return lz4f_stream_compress_alloc(&comp_ctx->u.lz4f_ctx, packet, len,
                                      complen, lz4f_net_compression_level);

  if (comp_ctx->algorithm == enum_compression_algorithm::MYSQL_ZSTD_STREAM)
    return zstd_stream_compress_alloc(&comp_ctx->u.zstd_ctx, packet, len,
                                      complen, zstd_net_compression_level);

  if (comp_ctx->algorithm == enum_compression_algorithm::MYSQL_ZSTD)
    return zstd_compress_alloc(&comp_ctx->u.zstd_ctx, packet, len, complen,
                               zstd_net_compression_level);

  if (comp_ctx->algorithm == enum_compression_algorithm::MYSQL_UNCOMPRESSED) {
    // If compression algorithm is set to none do not compress, even if compress
    // flag was set.
    *complen = 0;
    return nullptr;
  }

  DBUG_ASSERT(comp_ctx->algorithm == enum_compression_algorithm::MYSQL_ZLIB);
  return zlib_compress_alloc(&comp_ctx->u.zlib_ctx, packet, len, complen,
                             net_compression_level);
}

/*
  Uncompress packet

  @param comp_ctx      Pointer to compression context.
  @param packet        Compressed data. This is is replaced with the original
                       data.
  @param len           Length of compressed data
  @param complen [out] Length of the packet buffer after uncompression (must be
                       enough for the original data)

  @return true on error else false on success
*/

bool my_uncompress(mysql_compress_context *comp_ctx, uchar *packet, size_t len,
                   size_t *complen) {
  DBUG_ENTER("my_uncompress");
  DBUG_ASSERT(comp_ctx != nullptr);

  if (*complen) /* If compressed */
  {
    if (comp_ctx->algorithm == enum_compression_algorithm::MYSQL_ZSTD)
      DBUG_RETURN(zstd_uncompress(&comp_ctx->u.zstd_ctx, packet, len, complen));
    if (comp_ctx->algorithm == enum_compression_algorithm::MYSQL_ZSTD_STREAM)
      DBUG_RETURN(
          zstd_stream_uncompress(&comp_ctx->u.zstd_ctx, packet, len, complen));
    if (comp_ctx->algorithm == enum_compression_algorithm::MYSQL_LZ4F_STREAM)
      DBUG_RETURN(
          lz4f_stream_uncompress(&comp_ctx->u.lz4f_ctx, packet, len, complen));
    else if (comp_ctx->algorithm == enum_compression_algorithm::MYSQL_ZLIB)
      DBUG_RETURN(zlib_uncompress(packet, len, complen));
  } else {
    *complen = len;
    // On the compression side, the compression context is reset when an
    // uncompressed packet is sent. The same should be done on the
    // decompression side so that both contexts stay in sync.
    if (comp_ctx->algorithm == enum_compression_algorithm::MYSQL_ZSTD_STREAM) {
      if (comp_ctx->u.zstd_ctx.dctx) {
        ZSTD_DCtx_reset(comp_ctx->u.zstd_ctx.dctx, ZSTD_reset_session_only);
      }
    } else if (comp_ctx->algorithm ==
               enum_compression_algorithm::MYSQL_LZ4F_STREAM) {
      if (comp_ctx->u.lz4f_ctx.dctx) {
        DBUG_PRINT("note", ("lz4f_stream dctx reset"));
        LZ4F_resetDecompressionContext(comp_ctx->u.lz4f_ctx.dctx);
      }
    }
  }

  DBUG_RETURN(0);
}

/**
  Get default compression level corresponding to a given compression method.

  @param algorithm Compression Method. Possible values are zlib or zstd.

  @return an unsigned int representing default compression level.
          6 is the default compression level for zlib and 3 is the
          default compression level for zstd.
*/

unsigned int mysql_default_compression_level(
    enum enum_compression_algorithm algorithm) {
  switch (algorithm) {
    case MYSQL_ZLIB:
      return 6;
    case MYSQL_ZSTD:
    case MYSQL_ZSTD_STREAM:
      return 3;
    case MYSQL_LZ4F_STREAM:
      return 0;
    default:
      DBUG_ASSERT(0);  // should not reach here.
      return 0;        // To make compiler happy.
  }
}
