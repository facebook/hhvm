/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef STREAM_CIPHER_INCLUDED
#define STREAM_CIPHER_INCLUDED

#include <openssl/evp.h>
#include <memory>
#include <string>

/**
 openssl/aes.h define a macro:
  # define AES_BLOCK_SIZE 16
  This macro conflict with AES_BLOCK_SIZE variable definition in this file
 */
#if defined(AES_BLOCK_SIZE)
#pragma push_macro("AES_BLOCK_SIZE")
#undef AES_BLOCK_SIZE
#endif
/**
  @file stream_cipher.h

  @brief This file includes core components for encrypting/decrypting
         binary log files.
*/

typedef std::basic_string<unsigned char> Key_string;

/**
  @class Stream_cipher

  This abstract class represents the interface of a replication logs encryption
  cipher that can be used to encrypt/decrypt a given stream content in both
  sequential and random way.

  - Sequential means encrypting/decrypting a stream from the begin to end
    in order. For sequential encrypting/decrypting, you just need to call
    it like:

      open();
      encrypt();
      ...
      encrypt(); // call it again and again
      ...
      close();

  - Random means encrypting/decrypting a stream data without order. For
    example:

    - It first encrypts the data of a stream at the offset from 100 to 200.

    - And then encrypts the data of the stream at the offset from 0 to 99.

    For random encrypting/decrypting, you need to call set_stream_offset()
    before calling encrypt(). Example:

      open();

      set_stream_offset(100);
      encrypt(...);
      ...
      set_stream_offset(0);
      encrypt(...)

      close();
*/
class Stream_cipher {
 public:
  virtual ~Stream_cipher() {}

  /**
    Open the cipher with given password.

    @param[in] password The password which is used to initialize the cipher.
    @param[in] header_size The encrypted stream offset wrt the down stream.

    @retval false Success.
    @retval true Error.
  */
  virtual bool open(const Key_string &password, int header_size) = 0;

  /** Close the cipher. */
  virtual void close() = 0;

  /**
    Encrypt data.

    @param[in] dest The buffer for storing encrypted data. It should be
                    at least 'length' bytes.
    @param[in] src The data which will be encrypted.
    @param[in] length Length of the data.

    @retval false Success.
    @retval true Error.
  */
  virtual bool encrypt(unsigned char *dest, const unsigned char *src,
                       int length) = 0;

  /**
    Decrypt data.

    @param[in] dest The buffer for storing decrypted data. It should be
                    at least 'length' bytes.
    @param[in] src The data which will be decrypted.
    @param[in] length Length of the data.

    @retval false Success.
    @retval true Error.
  */
  virtual bool decrypt(unsigned char *dest, const unsigned char *src,
                       int length) = 0;

  /**
    Support encrypting/decrypting data at random position of a stream.

    @param[in] offset The stream offset of the data which will be encrypted/
                      decrypted in next encrypt()/decrypt() call.

    @retval false Success.
    @retval true Error.
  */
  virtual bool set_stream_offset(uint64_t offset) = 0;

  /**
    Returns the size of the header of the stream being encrypted/decrypted.

    @return the size of the header of the stream being encrypted/decrypted.
  */
  int get_header_size();

 protected:
  int m_header_size = 0;
};

/**
  @class Aes_ctr

  The class provides standards to be used by the Aes_ctr ciphers.
*/
class Aes_ctr {
 public:
  static const int PASSWORD_LENGTH = 32;
  static const int AES_BLOCK_SIZE = 16;
  static const int FILE_KEY_LENGTH = 32;
  /**
    Returns the message digest function to be uses when opening the cipher.

    @return SHA-512 message digest.
  */
  static const EVP_MD *get_evp_md() { return EVP_sha512(); }
  /**
    Returns the cipher to be uses when using the cipher.

    @return AES-256-CTR.
  */
  static const EVP_CIPHER *get_evp_cipher() { return EVP_aes_256_ctr(); }
  /**
    Returns a new unique Stream_cipher encryptor.

    @return A new Stream_cipher encryptor.
  */
  static std::unique_ptr<Stream_cipher> get_encryptor();
  /**
    Returns a new unique Stream_cipher decryptor.

    @return A new Stream_cipher decryptor.
  */
  static std::unique_ptr<Stream_cipher> get_decryptor();
};

enum class Cipher_type : int { ENCRYPT = 0, DECRYPT = 1 };

/**
  @class Aes_ctr_cipher

  The class implements AES-CTR encryption/decryption. It supports to
  encrypt/decrypt a stream in both sequential and random way.
*/
template <Cipher_type TYPE>
class Aes_ctr_cipher : public Stream_cipher {
 public:
  static const int PASSWORD_LENGTH = Aes_ctr::PASSWORD_LENGTH;
  static const int AES_BLOCK_SIZE = Aes_ctr::AES_BLOCK_SIZE;
  static const int FILE_KEY_LENGTH = Aes_ctr::FILE_KEY_LENGTH;

  virtual ~Aes_ctr_cipher() override;

  bool open(const Key_string &password, int header_size) override;
  void close() override;
  bool encrypt(unsigned char *dest, const unsigned char *src,
               int length) override;
  bool decrypt(unsigned char *dest, const unsigned char *src,
               int length) override;
  bool set_stream_offset(uint64_t offset) override;

 private:
  /* Cipher context */
  EVP_CIPHER_CTX *m_ctx = nullptr;
  /* The file key to encrypt/decrypt data. */
  unsigned char m_file_key[FILE_KEY_LENGTH];
  /* The initialization vector (IV) used to encrypt/decrypt data. */
  unsigned char m_iv[AES_BLOCK_SIZE];

  /**
    Initialize OpenSSL cipher related context and IV.

    @param[in] offset The stream offset to compute the AES-CTR counter which
                      will be set into IV.

    @retval false Success.
    @retval true Error.
  */
  bool init_cipher(uint64_t offset);

  /** Destroy OpenSSL cipher related context. */
  void deinit_cipher();
};

typedef class Aes_ctr_cipher<Cipher_type::ENCRYPT> Aes_ctr_encryptor;
typedef class Aes_ctr_cipher<Cipher_type::DECRYPT> Aes_ctr_decryptor;

#if defined(AES_BLOCK_SIZE)
#pragma pop_macro("AES_BLOCK_SIZE")
#endif
#endif  // STREAM_CIPHER_INCLUDED
