/*
Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef I_SHA2_PASSWORD_INCLUDED
#define I_SHA2_PASSWORD_INCLUDED

#include <string>
#include <unordered_map>

#include "crypt_genhash_impl.h"     /* For salt, sha2 digest */
#include "mysql/plugin.h"           /* MYSQL_PLUGIN */
#include "mysql/psi/mysql_rwlock.h" /* mysql_rwlock_t */
#include "sql/auth/i_sha2_password_common.h"

/**
  @file sql/auth/i_sha2_password.h
  Classes for caching_sha2_authentication plugin
*/

/**
  @defgroup auth_caching_sha2_auth caching_sha2_authentication information
  @{
*/
namespace sha2_password {
/* fast digest rounds */
const unsigned int MIN_FAST_DIGEST_ROUNDS = 2;
const unsigned int DEFAULT_FAST_DIGEST_ROUNDS = 2;
const unsigned int MAX_FAST_DIGEST_ROUNDS = 1000;

/* Length of Digest Info field */
const unsigned int DIGEST_INFO_LENGTH = 1;
/* Length of iteration info field */
const unsigned int ITERATION_LENGTH = 3;
/* Iteration multipler to be used on extracted iteration count */
const unsigned int ITERATION_MULTIPLIER = 1000;
/* Upper cap on iterations */
const long unsigned int MAX_ITERATIONS = 100000;
/* length of salt */
const unsigned int SALT_LENGTH = CRYPT_SALT_LENGTH;
/* $ + A + $ + ITERATION_LENGTH + $ + SALT_LENGTH + CACHING_SHA2_DIGEST_LENGTH =
 * 59 */
const unsigned int SHA256_AUTH_STRING_LEN =
    1 + 1 + 1 + ITERATION_LENGTH + 1 + SALT_LENGTH + CACHING_SHA2_DIGEST_LENGTH;
/* Delimiter character */
const char DELIMITER = '$';
/* Store digest length */
const unsigned int STORED_SHA256_DIGEST_LENGTH = 43;
/* stored digest rounds*/
const size_t MIN_STORED_DIGEST_ROUNDS = ROUNDS_MIN;
const size_t DEFAULT_STORED_DIGEST_ROUNDS = ROUNDS_DEFAULT;
const size_t MAX_STORED_DIGEST_ROUNDS = ROUNDS_MAX;
/* Maximum password length */
const size_t CACHING_SHA2_PASSWORD_MAX_PASSWORD_LENGTH = MAX_PLAINTEXT_LENGTH;
/* Maximum supported passwords */
const unsigned int MAX_PASSWORDS = 2;

typedef struct sha2_cache_entry {
  unsigned char digest_buffer[MAX_PASSWORDS][CACHING_SHA2_DIGEST_LENGTH];
} sha2_cache_entry;

/**
  Password cache used for caching_sha2_authentication
*/

class SHA2_password_cache {
 public:
  typedef std::unordered_map<std::string, sha2_cache_entry> password_cache;

  SHA2_password_cache() {}
  ~SHA2_password_cache();
  bool add(const std::string authorization_id,
           const sha2_cache_entry &entry_to_be_cached);
  bool remove(const std::string authorization_id);
  bool search(const std::string authorization_id,
              sha2_cache_entry &cache_entry);
  /** Returns number of cache entries present  */
  size_t size() { return m_password_cache.size(); }
  void clear_cache();

 private:
  password_cache m_password_cache;
};

/**
  Class to handle caching_sha2_authentication
  Provides methods for:
  - Fast authentication
  - Strong authentication
  - Removal of cached entry
*/
class Caching_sha2_password {
 public:
  Caching_sha2_password(
      MYSQL_PLUGIN plugin_handle,
      size_t stored_digest_rounds = DEFAULT_STORED_DIGEST_ROUNDS,
      unsigned int fast_digest_rounds = DEFAULT_FAST_DIGEST_ROUNDS,
      Digest_info digest_type = Digest_info::SHA256_DIGEST);
  ~Caching_sha2_password();
  std::pair<bool, bool> authenticate(const std::string &authorization_id,
                                     const std::string *serialized_string,
                                     const std::string &plaintext_password);
  std::pair<bool, bool> fast_authenticate(const std::string &authorization_id,
                                          const unsigned char *random,
                                          unsigned int random_length,
                                          const unsigned char *scramble,
                                          bool check_second);
  void remove_cached_entry(const std::string authorization_id);
  bool deserialize(const std::string &serialized_string,
                   Digest_info &digest_type, std::string &salt,
                   std::string &digest, size_t &iterations);
  bool serialize(std::string &serialized_string, const Digest_info &digest_type,
                 const std::string &salt, const std::string &digest,
                 size_t iterations);
  bool generate_fast_digest(const std::string &plaintext_password,
                            sha2_cache_entry &digest, unsigned int loc);
  bool generate_sha2_multi_hash(const std::string &src,
                                const std::string &random, std::string *digest,
                                unsigned int iterations);
  size_t get_cache_count();
  void clear_cache();
  bool validate_hash(const std::string serialized_string);
  Digest_info get_digest_type() const { return m_digest_type; }
  size_t get_digest_rounds() { return m_stored_digest_rounds; }

 private:
  /** Plugin handle */
  MYSQL_PLUGIN m_plugin_info;
  /** Number of rounds for stored digest */
  size_t m_stored_digest_rounds;
  /** Number of rounds for fast digest */
  unsigned int m_fast_digest_rounds;
  /** Digest type */
  Digest_info m_digest_type;
  /** Lock to protect @c m_cache */
  mysql_rwlock_t m_cache_lock;
  /** user=>password cache */
  SHA2_password_cache m_cache;
};
}  // namespace sha2_password

/** @} (end of auth_caching_sha2_auth) */

#endif  // !I_SHA2_PASSWORD_INCLUDED
