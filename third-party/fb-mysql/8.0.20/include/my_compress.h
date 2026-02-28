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

#ifndef MY_COMPRESS_INCLUDED
#define MY_COMPRESS_INCLUDED

/* List of valid values for compression_algorithm */
enum enum_compression_algorithm {
  MYSQL_UNCOMPRESSED = 1,
  MYSQL_ZLIB,
  MYSQL_ZSTD,
  MYSQL_ZSTD_STREAM,
  MYSQL_LZ4F_STREAM,
  MYSQL_INVALID
};

/**
  Compress context information. relating to zlib compression.
*/

typedef struct mysql_zlib_compress_context {
  /**
    Compression level to use in zlib compression.
  */
  unsigned int compression_level;
} mysql_zlib_compress_context;

typedef struct LZ4F_cctx_s LZ4F_compressionContext;
typedef struct LZ4F_dctx_s LZ4F_decompressionContext;

/**
  Compress context information. relating to LZ4 compression.
*/

typedef struct mysql_lz4f_compress_context {
  /**
    Compression level to use in lz4 compression.
  */
  LZ4F_compressionContext *cctx;
  LZ4F_decompressionContext *dctx;
  bool reset_cctx;
  unsigned char *compress_buf;
  unsigned long compress_buf_len;
  unsigned int compression_level;
} mysql_lz4f_compress_context;

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;

/**
  Compress context information relating to zstd compression.
*/

typedef struct mysql_zstd_compress_context {
  /**
    Pointer to compressor context.
  */
  ZSTD_CCtx *cctx;
  /**
    Pointer to decompressor context.
  */
  ZSTD_DCtx *dctx;
  /**
    Compression level to use in zstd compression.
  */
  bool reset_cctx;
  unsigned char *compress_buf;
  unsigned long compress_buf_len;
  unsigned int compression_level;
} mysql_zstd_compress_context;

/**
  Compression context information.
  It encapsulate the context information based on compression method and
  presents a generic struct.
*/

typedef struct mysql_compress_context {
  enum enum_compression_algorithm algorithm;  ///< Compression algorithm name.
  union {
    mysql_lz4f_compress_context lz4f_ctx;  ///< Context information of lz4.
    mysql_zlib_compress_context zlib_ctx;  ///< Context information of zlib.
    mysql_zstd_compress_context zstd_ctx;  ///< Context information of zstd.
  } u;
} mysql_compress_context;

/**
  Get default compression level corresponding to a given compression method.

  @param algorithm Compression Method. Possible values are zlib or zstd.

  @return an unsigned int representing default compression level.
  6 is the default compression level for zlib and 3 is the
  default compression level for zstd.
*/

unsigned int mysql_default_compression_level(
    enum enum_compression_algorithm algorithm);

/**
  Initialize a compress context object to be associated with a NET object.

  @param cmp_ctx Pointer to compression context.
  @param algorithm Compression algorithm.
  @param compression_level Compression level corresponding to the compression
  algorithm.
*/

void mysql_compress_context_init(mysql_compress_context *cmp_ctx,
                                 enum enum_compression_algorithm algorithm,
                                 unsigned int compression_level);
/**
  Deinitialize the compression context allocated.

  @param mysql_compress_ctx Pointer to Compression context.
*/

void mysql_compress_context_deinit(mysql_compress_context *mysql_compress_ctx);

void reset_compress_status(void);

#endif  // MY_COMPRESS_INCLUDED
