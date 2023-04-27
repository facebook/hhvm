/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file
  Wrapper functions for OpenSSL implementations.
*/

#include "my_inttypes.h"
#include "sha1.h"

#include <openssl/evp.h>
#include <openssl/sha.h>

/**
  Wrapper function to compute SHA1 message digest.

  @param [out] digest  Computed SHA1 digest
  @param [in] buf      Message to be computed
  @param [in] len      Length of the message
*/
void compute_sha1_hash(uint8 *digest, const char *buf, size_t len) {
  EVP_MD_CTX *sha1_context = EVP_MD_CTX_create();
  EVP_DigestInit_ex(sha1_context, EVP_sha1(), nullptr);
  EVP_DigestUpdate(sha1_context, buf, len);
  EVP_DigestFinal_ex(sha1_context, digest, nullptr);
  EVP_MD_CTX_destroy(sha1_context);
  sha1_context = nullptr;
}

/**
  Wrapper function to compute SHA1 message digest for
  two messages in order to emulate sha1(msg1, msg2).

  @param [out] digest  Computed SHA1 digest
  @param [in] buf1     First message
  @param [in] len1     Length of first message
  @param [in] buf2     Second message
  @param [in] len2     Length of second message
*/
void compute_sha1_hash_multi(uint8 *digest, const char *buf1, int len1,
                             const char *buf2, int len2) {
  EVP_MD_CTX *sha1_context = EVP_MD_CTX_create();
  EVP_DigestInit_ex(sha1_context, EVP_sha1(), nullptr);
  EVP_DigestUpdate(sha1_context, buf1, len1);
  EVP_DigestUpdate(sha1_context, buf2, len2);
  EVP_DigestFinal_ex(sha1_context, digest, nullptr);
  EVP_MD_CTX_destroy(sha1_context);
  sha1_context = nullptr;
}
