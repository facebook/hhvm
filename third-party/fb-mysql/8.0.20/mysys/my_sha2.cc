/* Copyright (c) 2007, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_sha2.cc
  A compatibility layer to our built-in SSL implementation, to mimic the
  oft-used external library, OpenSSL.
*/

#include "sha2.h"

/*  Low level digest API's are not allowed to access when FIPS mode is ON. This
 * wrapper will allow to call different sha256 methods directly.*/
#define GEN_OPENSSL_EVP_SHA2_BRIDGE(size)                          \
  unsigned char *SHA_EVP##size(const unsigned char *input_ptr,     \
                               size_t input_length,                \
                               char unsigned *output_ptr) {        \
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_create();                      \
    EVP_DigestInit_ex(md_ctx, EVP_sha##size(), NULL);              \
    EVP_DigestUpdate(md_ctx, input_ptr, input_length);             \
    EVP_DigestFinal_ex(md_ctx, (unsigned char *)output_ptr, NULL); \
    EVP_MD_CTX_destroy(md_ctx);                                    \
    return (output_ptr);                                           \
  }

/*
  @fn SHA_EVP512
  @fn SHA_EVP384
  @fn SHA_EVP256
  @fn SHA_EVP224
*/

GEN_OPENSSL_EVP_SHA2_BRIDGE(512)
GEN_OPENSSL_EVP_SHA2_BRIDGE(384)
GEN_OPENSSL_EVP_SHA2_BRIDGE(256)
GEN_OPENSSL_EVP_SHA2_BRIDGE(224)
#undef GEN_OPENSSL_EVP_SHA2_BRIDGE
