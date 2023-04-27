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

#ifndef BINLOG_OSTREAM_INCLUDED
#define BINLOG_OSTREAM_INCLUDED

#include <openssl/evp.h>
#include "sql/basic_ostream.h"
#include "sql/rpl_log_encryption.h"

// True if binlog cache is reset.
#ifndef DBUG_OFF
extern bool binlog_cache_is_reset;
#endif

/**
   Copy data from an input stream to an output stream.

   @param[in] istream   the input stream where data will be copied from
   @param[out] ostream  the output stream where data will be copied into
   @param[out] ostream_error It will be set to true if an error happens on
                             ostream and the pointer is not null. It is valid
                             only when the function returns true.

   @retval false Success
   @retval true Error happens in either the istream or ostream.
*/
template <class ISTREAM, class OSTREAM>
bool stream_copy(ISTREAM *istream, OSTREAM *ostream,
                 bool *ostream_error = nullptr) {
  unsigned char *buffer = nullptr;
  my_off_t length = 0;

  bool ret = istream->begin(&buffer, &length);
  while (!ret && length > 0) {
    if (ostream->write(buffer, length)) {
      if (ostream_error != nullptr) *ostream_error = true;
      return true;
    }

    ret = istream->next(&buffer, &length);
  }
  return ret;
}

/**
   A binlog cache implementation based on IO_CACHE.
*/
class IO_CACHE_binlog_cache_storage : public Truncatable_ostream {
 public:
  IO_CACHE_binlog_cache_storage();
  IO_CACHE_binlog_cache_storage &operator=(
      const IO_CACHE_binlog_cache_storage &) = delete;
  IO_CACHE_binlog_cache_storage(const IO_CACHE_binlog_cache_storage &) = delete;
  ~IO_CACHE_binlog_cache_storage() override;

  /**
     Opens the binlog cache. It creates a memory buffer as long as cache_size.
     The buffer will be extended up to max_cache_size when writting data. The
     data exceeds max_cache_size will be writting into temporary file.

     @param[in] dir  Where the temporary file will be created
     @param[in] prefix  Prefix of the temporary file name
     @param[in] cache_size  Size of the memory buffer.
     @param[in] max_cache_size  Maximum size of the memory buffer
     @retval false  Success
     @retval true  Error
  */
  bool open(const char *dir, const char *prefix, my_off_t cache_size,
            my_off_t max_cache_size);
  void close();

  bool write(const unsigned char *buffer, my_off_t length) override;
  bool truncate(my_off_t offset) override;
  /* purecov: inspected */
  /* binlog cache doesn't need seek operation. Setting true to return error */
  bool seek(my_off_t offset MY_ATTRIBUTE((unused))) override { return true; }
  /**
     Reset status and drop all data. It looks like a cache never was used after
     reset.
  */
  bool reset();
  /**
     Returns the file name if a temporary file is opened, otherwise nullptr is
     returned.
  */
  const char *tmp_file_name() const;
  /**
     Returns the count of calling temporary file's write()
  */
  size_t disk_writes() const;

  /**
     Initializes binlog cache for reading and returns the data at the begin.
     buffer is controlled by binlog cache implementation, so caller should
     not release it. If the function sets *length to 0 and no error happens,
     it has reached the end of the cache.

     @param[out] buffer  It points to buffer where data is read.
     @param[out] length  Length of the data in the buffer.
     @retval false  Success
     @retval true  Error
  */
  bool begin(unsigned char **buffer, my_off_t *length);
  /**
     Returns next piece of data. buffer is controlled by binlog cache
     implementation, so caller should not release it. If the function sets
     *length to 0 and no error happens, it has reached the end of the cache.

     @param[out] buffer  It points to buffer where data is read.
     @param[out] length  Length of the data in the buffer.
     @retval false  Success
     @retval true  Error
  */
  bool next(unsigned char **buffer, my_off_t *length);
  my_off_t length() const;
  bool flush() override { return false; }
  bool sync() override { return false; }

  /**
     Return the underlying io_cache for this stream object
     @retval A pointer to the underlying IO_CACHE
  */
  IO_CACHE *get_io_cache() { return &m_io_cache; }

 private:
  IO_CACHE m_io_cache;
  my_off_t m_max_cache_size = 0;
  /**
    Enable IO Cache temporary file encryption.

    @retval false Success.
    @retval true Error.
  */
  bool enable_encryption();
  /**
    Disable IO Cache temporary file encryption.
  */
  void disable_encryption();
  /**
    Generate a new password for the temporary file encryption.

    This function is called by reset() that is called every time a transaction
    commits to cleanup the binary log cache. The file password shall vary not
    only per temporary file, but also per transaction being committed within a
    single client connection.

    @retval false Success.
    @retval true Error.
  */
  bool setup_ciphers_password();
};

/**
   Byte container that provides a storage for serializing session
   binlog events. This way of arranging the classes separates storage layer
   and binlog layer, hides the implementation detail of low level storage.
*/
class Binlog_cache_storage : public Basic_ostream {
 public:
  ~Binlog_cache_storage() override;

  bool open(my_off_t cache_size, my_off_t max_cache_size);
  void close();

  bool write(const unsigned char *buffer, my_off_t length) override {
    DBUG_ASSERT(m_pipeline_head != nullptr);
    return m_pipeline_head->write(buffer, length);
  }
  /**
     Truncates some data at the end of the binlog cache.

     @param[in] offset  Where the binlog cache will be truncated to.
     @retval false  Success
     @retval true  Error
  */
  bool truncate(my_off_t offset) { return m_pipeline_head->truncate(offset); }

  /**
     Reset status and drop all data. It looks like a cache was never used
     after reset.
  */
  bool reset() { return m_file.reset(); }
  /**
     Returns the count of disk writes
  */
  size_t disk_writes() const { return m_file.disk_writes(); }
  /**
     Returns the name of the temporary file.
  */
  const char *tmp_file_name() const { return m_file.tmp_file_name(); }

  /**
     Copy all data to a output stream. This function hides the internal
     implementation of storage detail. So it will not disturb the callers
     if the implementation of Binlog_cache_storage is changed. If we add
     a pipeline stream in this class, then we need to change the implementation
     of this function. But callers are not affected.

     @param[out] ostream Where the data will be copied into
     @param[out] ostream_error It will be set to true if an error happens on
                               ostream and the pointer is not null. It is valid
                               only when the function returns true.
     @retval false  Success
     @retval true  Error happens in either the istream or ostream.
  */
  bool copy_to(Basic_ostream *ostream, bool *ostream_error = nullptr) {
    return stream_copy(&m_file, ostream, ostream_error);
  }

  /**
     Returns data length.
  */
  my_off_t length() const { return m_file.length(); }
  /**
     Returns true if binlog cache is empty.
  */
  bool is_empty() const { return length() == 0; }

  /**
     Return the underlying io_cache for this stream object
     @retval A pointer to the underlying IO_CACHE
  */
  IO_CACHE *get_io_cache() { return m_file.get_io_cache(); }

 private:
  Truncatable_ostream *m_pipeline_head = nullptr;
  IO_CACHE_binlog_cache_storage m_file;
};

/**
  It is an Truncatable_ostream which provides encryption feature. It can be
  setup into an stream pipeline. In the pipeline, it encrypts the data
  from up stream and then feeds the encrypted data into down stream.
*/
class Binlog_encryption_ostream : public Truncatable_ostream {
 public:
  ~Binlog_encryption_ostream() override;

  /**
    Initialize the context used in the encryption stream and write encryption
    header into down stream.

    @param[in] down_ostream The stream for storing encrypted data.

    @retval false Success
    @retval true Error.
  */
  bool open(std::unique_ptr<Truncatable_ostream> down_ostream);

  /**
    Initialize the context used in the encryption stream based on the
    header passed as parameter. It shall be used when opening an ostream for
    a stream that was already encrypted (the cypher password already exists).

    @param[in] down_ostream the stream for storing encrypted data.
    @param[in] header the encryption header to setup the cypher.

    @retval false Success.
    @retval true Error.
  */
  bool open(std::unique_ptr<Truncatable_ostream> down_ostream,
            std::unique_ptr<Rpl_encryption_header> header);

  /**
    Re-encrypt the encrypted binary/relay log file header by replacing its
    binlog encryption key id with the current one and its encrypted file
    password with the new one, which is got by encrypting its file password
    with the current binlog encryption key.

    @retval false Success with an empty error message.
    @retval true Error with an error message.
  */
  std::pair<bool, std::string> reencrypt();

  void close();
  bool write(const unsigned char *buffer, my_off_t length) override;
  bool truncate(my_off_t offset) override;
  bool seek(my_off_t offset) override;
  bool flush() override;
  bool sync() override;
  /**
    Return the encrypted file header size.

    @return the encrypted file header size.
  */
  int get_header_size();

  my_off_t get_my_b_tell() override { return m_down_ostream->get_my_b_tell(); }

 private:
  std::unique_ptr<Truncatable_ostream> m_down_ostream;
  std::unique_ptr<Rpl_encryption_header> m_header;
  std::unique_ptr<Stream_cipher> m_encryptor;
};
#endif  // BINLOG_OSTREAM_INCLUDED
