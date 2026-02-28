/*
   Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* password checking routines */
/*****************************************************************************
  The main idea is that no password are sent between client & server on
  connection and that no password are saved in mysql in a decodable form.

  On connection a random string is generated and sent to the client.
  The client generates a new string with a random generator inited with
  the hash values from the password and the sent string.
  This 'check' string is sent to the server where it is compared with
  a string generated from the stored hash_value of the password and the
  random string.

  The password is saved (in user.authentication_string).

  Example:
    SET PASSWORD for test = 'haha'
  This saves a hashed number as a string in the authentication_string field.

  The new authentication is performed in following manner:

  SERVER:  public_seed=generate_user_salt()
           send(public_seed)

  CLIENT:  recv(public_seed)
           hash_stage1=sha1("password")
           hash_stage2=sha1(hash_stage1)
           reply=xor(hash_stage1, sha1(public_seed,hash_stage2)

           // this three steps are done in scramble()

           send(reply)


  SERVER:  recv(reply)
           hash_stage1=xor(reply, sha1(public_seed,hash_stage2))
           candidate_hash2=sha1(hash_stage1)
           check(candidate_hash2==hash_stage2)

           // this three steps are done in check_scramble()

*****************************************************************************/

#include <string.h>
#include <sys/types.h>

#include "crypt_genhash_impl.h"
#include "m_string.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "mysql_com.h"
#include "sha1.h"

void randominit(struct rand_struct *rand_st, ulong seed1,
                ulong seed2) { /* For mysql 3.21.# */
  rand_st->max_value = 0x3FFFFFFFL;
  rand_st->max_value_dbl = (double)rand_st->max_value;
  rand_st->seed1 = seed1 % rand_st->max_value;
  rand_st->seed2 = seed2 % rand_st->max_value;
}

/*
    Generate binary hash from raw text string
    Used for Pre-4.1 password handling
  SYNOPSIS
    hash_password()
    result       OUT store hash in this location
    password     IN  plain text password to build hash
    password_len IN  password length (password may be not null-terminated)
*/

void hash_password(ulong *result, const char *password, uint password_len) {
  ulong nr = 1345345333L, add = 7, nr2 = 0x12345671L;
  ulong tmp;
  const char *password_end = password + password_len;
  for (; password < password_end; password++) {
    if (*password == ' ' || *password == '\t')
      continue; /* skip space in password */
    tmp = (ulong)(uchar)*password;
    nr ^= (((nr & 63) + add) * tmp) + (nr << 8);
    nr2 += (nr2 << 8) ^ nr;
    add += tmp;
  }
  result[0] = nr & (((ulong)1L << 31) - 1L); /* Don't use sign bit (str2int) */
  ;
  result[1] = nr2 & (((ulong)1L << 31) - 1L);
}

static inline uint8 char_val(uint8 X) {
  return (uint)(X >= '0' && X <= '9'
                    ? X - '0'
                    : X >= 'A' && X <= 'Z' ? X - 'A' + 10 : X - 'a' + 10);
}

/* Character to use as version identifier for version 4.1 */

#define PVERSION41_CHAR '*'

/*
    Convert given octet sequence to asciiz string of hex characters;
    str..str+len and 'to' may not overlap.
  SYNOPSIS
    octet2hex()
    buf       OUT output buffer. Must be at least 2*len+1 bytes
    str, len  IN  the beginning and the length of the input string

  RETURN
    buf+len*2
*/

char *octet2hex(char *to, const char *str, uint len) {
  const char *str_end = str + len;
  for (; str != str_end; ++str) {
    *to++ = _dig_vec_upper[((uchar)*str) >> 4];
    *to++ = _dig_vec_upper[((uchar)*str) & 0x0F];
  }
  *to = '\0';
  return to;
}

/*
    Convert given asciiz string of hex (0..9 a..f) characters to octet
    sequence.
  SYNOPSIS
    hex2octet()
    to        OUT buffer to place result; must be at least len/2 bytes
    str, len  IN  begin, length for character string; str and to may not
                  overlap; len % 2 == 0
*/

static void hex2octet(uint8 *to, const char *str, uint len) {
  const char *str_end = str + len;
  while (str < str_end) {
    char tmp = char_val(*str++);
    *to++ = (tmp << 4) | char_val(*str++);
  }
}

/*
    Encrypt/Decrypt function used for password encryption in authentication.
    Simple XOR is used here but it is OK as we crypt random strings. Note,
    that XOR(s1, XOR(s1, s2)) == s2, XOR(s1, s2) == XOR(s2, s1)
  SYNOPSIS
    my_crypt()
    to      OUT buffer to hold crypted string; must be at least len bytes
                long; to and s1 (or s2) may be the same.
    s1, s2  IN  input strings (of equal length)
    len     IN  length of s1 and s2
*/

static void my_crypt(char *to, const uchar *s1, const uchar *s2, uint len) {
  const uint8 *s1_end = s1 + len;
  while (s1 < s1_end) *to++ = *s1++ ^ *s2++;
}

extern "C" void my_make_scrambled_password(char *to, const char *password,
                                           size_t pass_len) {
  char salt[CRYPT_SALT_LENGTH + 1];

  generate_user_salt(salt, CRYPT_SALT_LENGTH + 1);
  my_crypt_genhash(to, CRYPT_MAX_PASSWORD_SIZE, password, pass_len, salt,
                   nullptr);
}

/**
  Compute two stage SHA1 hash of the password :

    hash_stage1=sha1("password")
    hash_stage2=sha1(hash_stage1)

  @param [in] password       Password string.
  @param [in] pass_len       Length of the password.
  @param [out] hash_stage1   sha1(password)
  @param [out] hash_stage2   sha1(hash_stage1)
*/

inline static void compute_two_stage_sha1_hash(const char *password,
                                               size_t pass_len,
                                               uint8 *hash_stage1,
                                               uint8 *hash_stage2) {
  /* Stage 1: hash password */
  compute_sha1_hash(hash_stage1, password, pass_len);

  /* Stage 2 : hash first stage's output. */
  compute_sha1_hash(hash_stage2, (const char *)hash_stage1, SHA1_HASH_SIZE);
}

/*
    MySQL 4.1.1 password hashing: SHA conversion (see RFC 2289, 3174) twice
    applied to the password string, and then produced octet sequence is
    converted to hex string.
    The result of this function is stored in the database.
  SYNOPSIS
    my_make_scrambled_password_sha1()
    buf       OUT buffer of size 2*SHA1_HASH_SIZE + 2 to store hex string
    password  IN  password string
    pass_len  IN  length of password string
*/

void my_make_scrambled_password_sha1(char *to, const char *password,
                                     size_t pass_len) {
  uint8 hash_stage2[SHA1_HASH_SIZE];

  /* Two stage SHA1 hash of the password. */
  compute_two_stage_sha1_hash(password, pass_len, (uint8 *)to, hash_stage2);

  /* convert hash_stage2 to hex string */
  *to++ = PVERSION41_CHAR;
  octet2hex(to, (const char *)hash_stage2, SHA1_HASH_SIZE);
}

/*
  Wrapper around my_make_scrambled_password() to maintain client lib ABI
  compatibility.
  In server code usage of my_make_scrambled_password() is preferred to
  avoid strlen().
  SYNOPSIS
    make_scrambled_password()
    buf       OUT buffer of size 2*SHA1_HASH_SIZE + 2 to store hex string
    password  IN  NULL-terminated password string
*/

void make_scrambled_password(char *to, const char *password) {
  my_make_scrambled_password_sha1(to, password, strlen(password));
}

/**
    Produce an obscure octet sequence from password and random
    string, received from the server. This sequence corresponds to the
    password, but password can not be easily restored from it. The sequence
    is then sent to the server for validation. Trailing zero is not stored
    in the buf as it is not needed.
    This function is used by client to create authenticated reply to the
    server's greeting.

    @param[out] to   store scrambled string here. The buf must be at least
                     SHA1_HASH_SIZE bytes long.
    @param message   random message, must be exactly SCRAMBLE_LENGTH long and
                     NULL-terminated.
    @param password  users' password, NULL-terminated
*/

void scramble(char *to, const char *message, const char *password) {
  uint8 hash_stage1[SHA1_HASH_SIZE];
  uint8 hash_stage2[SHA1_HASH_SIZE];

  /* Two stage SHA1 hash of the password. */
  compute_two_stage_sha1_hash(password, strlen(password), hash_stage1,
                              hash_stage2);

  /* create crypt string as sha1(message, hash_stage2) */;
  compute_sha1_hash_multi((uint8 *)to, message, SCRAMBLE_LENGTH,
                          (const char *)hash_stage2, SHA1_HASH_SIZE);
  my_crypt(to, (const uchar *)to, hash_stage1, SCRAMBLE_LENGTH);
}

/**
    Check that scrambled message corresponds to the password.

    The function is used by server to check that received reply is authentic.
    This function does not check lengths of given strings: message must be
    null-terminated, reply and hash_stage2 must be at least SHA1_HASH_SIZE
    long (if not, something fishy is going on).

    @param scramble_arg  clients' reply, presumably produced by scramble()
    @param message       original random string, previously sent to client
                         (presumably second argument of scramble()), must be
                         exactly SCRAMBLE_LENGTH long and NULL-terminated.
    @param hash_stage2   hex2octet-decoded database entry

    @retval false  password is correct
    Wretval true   password is invalid
*/

static bool check_scramble_sha1(const uchar *scramble_arg, const char *message,
                                const uint8 *hash_stage2) {
  uint8 buf[SHA1_HASH_SIZE];
  uint8 hash_stage2_reassured[SHA1_HASH_SIZE];

  /* create key to encrypt scramble */
  compute_sha1_hash_multi(buf, message, SCRAMBLE_LENGTH,
                          (const char *)hash_stage2, SHA1_HASH_SIZE);
  /* encrypt scramble */
  my_crypt((char *)buf, buf, scramble_arg, SCRAMBLE_LENGTH);

  /* now buf supposedly contains hash_stage1: so we can get hash_stage2 */
  compute_sha1_hash(hash_stage2_reassured, (const char *)buf, SHA1_HASH_SIZE);

  return (memcmp(hash_stage2, hash_stage2_reassured, SHA1_HASH_SIZE) != 0);
}

bool check_scramble(const uchar *scramble_arg, const char *message,
                    const uint8 *hash_stage2) {
  return check_scramble_sha1(scramble_arg, message, hash_stage2);
}

/*
  Convert scrambled password from asciiz hex string to binary form.

  SYNOPSIS
    get_salt_from_password()
    res       OUT buf to hold password. Must be at least SHA1_HASH_SIZE
                  bytes long.
    password  IN  4.1.1 version value of user.password
*/

void get_salt_from_password(uint8 *hash_stage2, const char *password) {
  hex2octet(hash_stage2, password + 1 /* skip '*' */, SHA1_HASH_SIZE * 2);
}

/**
  Convert scrambled password from binary form to asciiz hex string.

  @param [out] to     store resulting string here, 2*SHA1_HASH_SIZE+2 bytes
  @param hash_stage2  password in salt format
*/

void make_password_from_salt(char *to, const uint8 *hash_stage2) {
  *to++ = PVERSION41_CHAR;
  octet2hex(to, (const char *)hash_stage2, SHA1_HASH_SIZE);
}
