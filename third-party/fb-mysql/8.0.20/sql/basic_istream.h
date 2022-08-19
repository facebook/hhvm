/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef BASIC_ISTREAM_INCLUDED
#define BASIC_ISTREAM_INCLUDED
#include "my_io.h"
#include "my_sys.h"

/**
   The abstract class for basic byte input streams which provides read
   operations.
*/
class Basic_istream {
 public:
  /**
     Read some bytes from the input stream. It should read exact 'length' bytes
     unless error happens or it reaches the end of the stream. It should block
     when reaching the end of a pipe that is not closed.

     @param[out] buffer Where data will be put in.
     @param[in] length The number of bytes that you want to read. length should
                       not be larger than max long.
     @return Return values fall into three cases:
     @retval 'length' Read 'length' bytes successfully
     @retval >=0      Reach EOF, return the number of bytes actually read.
                      It is between 0 and length-1.
     @retval -1       Error.
  */
  virtual ssize_t read(unsigned char *buffer, size_t length) = 0;

  virtual ~Basic_istream() {}
};

/**
   The abstract class for seekable input streams which have fixed length
   and provide seek operation.
*/
class Basic_seekable_istream : public Basic_istream {
 public:
  /**
     Puts the read position to a given offset. The offset counts from the
     beginning of the stream. In case an implementing class transforms the data
     in a way that does not preserve positions, the offset here will be relative
     to the bytes that are read out from the stream, not relative to the bytes
     in lower layer storage.

     it is allowed for a subclass to return success even if the position is
     greater than the size of the file. Error may be returned by the next
     read for this case. Users should call length() if they need to check that
     the position is within bounds.

     @param[in] offset  Where the read position will be.
     @retval false  Success
     @retval true  Error
  */
  virtual bool seek(my_off_t offset) = 0;
  /**
     The total length of the stream.
   */
  virtual my_off_t length() = 0;
  virtual ~Basic_seekable_istream() {}
};

/**
   A file input stream based on IO_CACHE class. It can be used to open a file
   and provide a Basic_seekable_istream based on the file.
*/
class IO_CACHE_istream : public Basic_seekable_istream {
 public:
  IO_CACHE_istream();
  IO_CACHE_istream(const IO_CACHE_istream &) = delete;
  IO_CACHE_istream &operator=(const IO_CACHE_istream &) = delete;
  ~IO_CACHE_istream() override;

  /**
     Open the stream. It opens related file and initializes IO_CACHE.

     @param[in] log_file_key  The PSI_file_key for this stream
     @param[in] log_cache_key  The PSI_file_key for the IO_CACHE
     @param[in] file_name  The file to be opened
     @param[in] flags  The flags used by IO_CACHE.
     @param[in] cache_size  Cache size of the IO_CACHE.
     @retval false  Success
     @retval true  Error
  */
  bool open(
#ifdef HAVE_PSI_INTERFACE
      PSI_file_key log_file_key, PSI_file_key log_cache_key,
#endif
      const char *file_name, myf flags, size_t cache_size = IO_SIZE * 2);
  /**
    Closes the stream. It deinitializes IO_CACHE and closes the file it opened.
  */
  void close();

  ssize_t read(unsigned char *buffer, size_t length) override;
  bool seek(my_off_t bytes) override;

  /**
     Get the length of the file.
  */
  my_off_t length() override;

 private:
  IO_CACHE m_io_cache;
};

/**
   A stdin input stream based on IO_CACHE. It provides a Basic_istream based on
   stdin.
*/
class Stdin_istream : public Basic_istream {
 public:
  Stdin_istream();
  Stdin_istream(const Stdin_istream &) = delete;
  Stdin_istream &operator=(const Stdin_istream &) = delete;
  ~Stdin_istream() override;

  /**
     Opens the stdin stream. It initializes the IO_CACHE with stdin.
     @param[out] errmsg An error message is returned if any error happens.
  */
  bool open(std::string *errmsg);
  /**
     Closes the stream. It deinitializes IO_CACHE.
  */
  void close();

  ssize_t read(unsigned char *buffer, size_t length) override;
  /**
     Skip bytes data from the stdin stream.
     @param[in] bytes How many bytes should be skipped
   */
  bool skip(my_off_t bytes);

 private:
  IO_CACHE m_io_cache;
};

#endif  // BASIC_ISTREAM_INCLUDED
