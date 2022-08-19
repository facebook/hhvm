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

/**
  @addtogroup Replication
  @{

  @file event_reader.h

  @brief Contains the class responsible for deserializing fields of an event
         previously stored in a buffer.
*/

#ifndef EVENT_READER_INCLUDED
#define EVENT_READER_INCLUDED

#include <list>
#include <map>
#include <string>
#include <vector>
#include "byteorder.h"
#include "wrapper_functions.h"

namespace binary_log {

#define PRINT_READER_STATUS(message)               \
  BAPI_PRINT("debug", (message ": m_buffer= %p, "  \
                               "m_limit= %llu, "   \
                               "m_length= %llu, "  \
                               "position()= %llu", \
                       m_buffer, m_limit, m_length, Event_reader::position()))

/**
  Event_reader class purpose is to avoid out-of-buffer reads when deserializing
  binary log events and increase robustness when dealing with corrupted event
  buffers.

  The Event_reader is composed by a pointer to the beginning of the serialized
  event buffer (m_buffer), a variable containing the buffer length (m_length), a
  cursor pointer that tells the current position to be read from the buffer
  (m_ptr) and the buffer limit the reader shall respect (m_limit <= m_length).

  All buffer reading functions shall move the cursor forward.

  Before reading from the buffer, the Event_reader will check if the amount of
  bytes expected to be read are less or equal to the remaining bytes to read:

    remaining = m_limit - (m_ptr - m_buffer)

  When there are no enough bytes to read from the buffer, Event_reader enters
  in error state, so its owner can take an action.
*/

class Event_reader {
 public:
  /**
    Event_reader constructor.

    It sets the cursor to the first position of the buffer.

    @param[in] buffer buffer holding a serialized event
    @param[in] length known buffer length.
  */
  Event_reader(const char *buffer, unsigned long long length)
      : m_buffer(buffer),
        m_ptr(buffer),
        m_length(length),
        m_limit(length),
        m_error(nullptr) {}

  /**
    Returns if the Event_reader is in an error state or not.

    @retval true if the Event_reader is in error state.
    @retval false if the Event_reader is not in error state.
  */
  bool has_error() {
    BAPI_PRINT("debug", ("m_error= %s", m_error ? m_error : "nullptr"));
    return m_error != nullptr;
  }

  /**
    Returns the pointer to the error message.

    @return the pointer to the error message when Event_reader is in error
            state, or a nullptr otherwise.
  */
  const char *get_error() { return m_error; }

  /**
    Sets Event_reader error state by setting the error message.

    @param[in] error pointer to the error message.
  */
  void set_error(const char *error);

  /**
    Returns the Event_reader buffer length.

    Note: the buffer length might be larger than reader allowed buffer limit,
    but the Event_reader will enter error state when trying to read above the
    limit.

    Example: an event buffer may contain the serialized event + checksum. The
    event reader object will be configured with a buffer length that contains
    both the serialized event and the checksum information, but once
    Log_event_footer is instantiated, it shall adjust the event reader buffer
    limit to the buffer position right before the checksum. This will avoid some
    event deserialization relying on event buffer size to assume the checksum as
    serialized event content.

    @return the Event_reader buffer length.
  */
  unsigned long long length() { return (m_length); }

  /**
    Sets Event_reader buffer length and limit.

    The length of the buffer should only be set to values greater or equal to
    the current buffer length. Trying to set the length to less than current
    buffer length will make the Event_buffer to enter error state.

    The length is initially set in Event_reader constructor to
    LOG_EVENT_MINIMAL_HEADER_LEN by the Log_event_header when instantiating it.
    This should be enough to read the event header and determine the correct
    buffer length. The Log_event_header will adjust the Event_reader length by
    calling this function based on the value of event data_written header field.

    @param[in] length the new Event_reader buffer length.
  */
  void set_length(unsigned long long length);

  /**
    Shrinks the Event_reader buffer limit.

    This function is used by Log_event_footer to remove the checksum payload (if
    necessary) from the serialized event size, as many event types rely on the
    serialized event size to determine the size of some fields.

    @param[in] bytes the amount of bytes to shrink the Event_reader buffer
                     length.
  */
  void shrink_limit(unsigned long long bytes);

  /**
    Returns the Event_reader buffer pointer.

    @return the Event_reader buffer pointer.
  */
  const char *buffer() { return m_buffer; }

  /**
    Returns a pointer to the Event_reader cursor (next position to be read by
    the Event_reader functions).

    @return the pointer to the Event_reader cursor.
  */
  const char *ptr() { return m_ptr; }

  /**
    Returns a pointer to the Event_reader cursor (next position to be read) and
    moves the cursor forward.

    This function is used when the buffer contains a field of a known size and
    the deserialization procedure must keep the pointer to the field but moving
    the cursor to after it.

    @param[in] length the amount of bytes to move the cursor forward.

    @return the pointer to the Event_reader cursor before forwarding it.
  */
  const char *ptr(unsigned long long length);

  /**
    Returns the current Event_reader cursor position in bytes.

    @retval m_limit if cursor position is invalid.
    @retval position current Event_reader cursor position (if valid).
  */
  unsigned long long position() {
    return m_ptr >= m_buffer ? m_ptr - m_buffer : m_limit;
  }

  /**
    Returns the amount of bytes still available to read from cursor position.

    @return the amount of bytes still available to read.
  */
  unsigned long long available_to_read() {
    BAPI_ASSERT(position() <= m_limit);
    return m_limit - position();
  }

  /**
    Returns if the Event_reader can read a given amount of bytes from cursor
    position.

    @param bytes the amount of bytes expected to be read.

    @retval true if the Event_reader can read the specified amount of bytes.
    @retval false if the Event_reader cannot read the specified amount of bytes.
  */
  bool can_read(unsigned long long bytes) {
    return (available_to_read() >= bytes);
  }

  /**
    Moves cursor to a given absolute buffer position and returns the pointer to
    the cursor.

    @param position the position to jump to.

    @retval pointer a pointer to the new cursor position.
    @retval nullptr if the position is out of buffer boundaries.
  */
  const char *go_to(unsigned long long position);

  /**
    Moves the buffer position forward to a given relative position and returns
    the pointer to the buffer on the specified position.

    @param bytes the amount of bytes to move forward.

    @retval pointer a pointer to the new buffer position.
    @retval nullptr if the cursor is out of buffer boundaries.
  */
  const char *forward(unsigned long long bytes) {
    BAPI_PRINT("debug", ("Event_reader::forward(%llu)", bytes));
    return go_to((m_ptr - m_buffer) + bytes);
  }

  /**
    Reads a basic type - bool, char, int, long, double, etc - from the buffer,
    moves the cursor forward the number of bytes returned by sizeof(T)) and
    returns the read value.

    @retval value the T read from the cursor position.
    @retval 0 if the cursor was out of buffer boundaries.
  */
  template <class T>
  T read() {
    PRINT_READER_STATUS("Event_reader::read");
    if (!can_read(sizeof(T))) {
      set_error("Cannot read from out of buffer bounds");
      BAPI_PRINT("debug", ("Event_reader::tread(): "
                           "sizeof()= %zu",
                           sizeof(T)));
      return 0;
    }
    T value = 0;
    value = (T) * (m_ptr);
    m_ptr = m_ptr + sizeof(T);
    return value;
  }

  /**
    Copies a basic type - bool, char, int, long, double, etc - from the buffer,
    moves the cursor forward the number of bytes returned by sizeof(T)) and
    returns the copied value.

    @retval value the T copied from the cursor position.
    @retval 0 if the cursor was out of buffer boundaries.
  */
  template <class T>
  T memcpy() {
    PRINT_READER_STATUS("Event_reader::memcpy");
    if (!can_read(sizeof(T))) {
      set_error("Cannot read from out of buffer bounds");
      BAPI_PRINT("debug", ("Event_reader::memcpy(): "
                           "sizeof()= %zu",
                           sizeof(T)));
      return 0;
    }
    T value = 0;
    ::memcpy((char *)&value, m_ptr, sizeof(T));
    m_ptr = m_ptr + sizeof(T);
    return value;
  }

  /**
    Copies a basic arithmetic type - uint8_t, [u]int16_t, [u]int32_t,
    [u]int64_t - from the buffer, moves the cursor forward using specified bytes
    parameter (or the number of bytes returned by sizeof(T) when not specified)
    and returns the copied value transformed from little endian if necessary).

    @param[in] bytes the amount of bytes to read from the buffer (and to move
                     forward). When not specified, will use sizeof(T).

    @retval value the T copied from the cursor position.
    @retval 0 if the cursor was out of buffer boundaries or there was no memory
              to allocate to the new string..
  */
  template <typename T>
  T read_and_letoh(unsigned char bytes = sizeof(T)) {
    PRINT_READER_STATUS("Event_reader::read_and_letoh");
    if (!can_read(bytes)) {
      set_error("Cannot read from out of buffer bounds");
      BAPI_PRINT("debug", ("Event_reader::read_and_letoh(): "
                           "sizeof()= %zu, bytes= %u",
                           sizeof(T), bytes));
      return 0;
    }
    T value = 0;
    ::memcpy((char *)&value, m_ptr, bytes);
    m_ptr = m_ptr + bytes;
    return letoh(value);
  }

  /**
    Returns a pointer to a new string which is a duplicate of the input string.
    The terminating null character is added. See: bapi_strndup().

    @param[in] length the amount of bytes to read from the buffer (and to move
                      forward).

    @retval pointer the T pointer from the cursor position.
    @retval nullptr if the cursor was out of buffer boundaries.
  */
  template <typename T>
  T strndup(size_t length) {
    PRINT_READER_STATUS("Event_reader::strndup");
    if (!can_read(length)) {
      BAPI_PRINT("debug", ("Event_reader::strndup(%zu)", length));
      set_error("Cannot read from out of buffer bounds");
      return nullptr;
    }
    T str;
    str = reinterpret_cast<T>(bapi_strndup(m_ptr, length));
    m_ptr = m_ptr + length;
    return str;
  }

  /**
    Copies from the cursor to an already existent (and allocated) buffer and
    moves forward the cursor.

    @param[out] destination a pointer to the destination buffer.
    @param[in] length the amount of bytes to read from the buffer (and to move
                      forward).
  */
  template <typename T>
  void memcpy(T destination, size_t length) {
    PRINT_READER_STATUS("Event_reader::memcpy");
    if (!can_read(length)) {
      BAPI_PRINT("debug", ("Event_reader::memcpy(%zu)", length));
      set_error("Cannot read from out of buffer bounds");
      return;
    }
    ::memcpy(destination, m_ptr, length);
    m_ptr = m_ptr + length;
  }

  /**
    Allocates memory to a destination buffer, copies from the cursor to the
    destination buffer using memcpy() and moves forward the cursor.

    This function is useful for pairs of fields when a first field describes the
    second field size and the deserialization procedure must allocate a buffer
    for the second field and then copy the event buffer content to the new
    allocated buffer.

    Before implementing this function and the Event_reader, the deserialization
    process did like:

      memcpy(length, ptr, sizeof(length);
      ptr+=sizeof(length);
      field = malloc(length);
      memcpy(field, ptr, length);

    Allocating the memory for the field before knowing if the content can be
    read from the event buffer is a mistake, as it might allocate a very large
    amount of memory that will not be used.

    So, alloc_and_memcpy ensures that it will only allocate memory for the field
    if it can be read from the event buffer, avoiding allocating a memory that
    will not be used.

    @param[out] destination the destination buffer.
    @param[in] length the amount of bytes to allocate and read from the buffer
                      (and to move forward).
    @param[in] flags flags to pass to MySQL server my_malloc() function.
  */
  void alloc_and_memcpy(unsigned char **destination, size_t length, int flags);

  /**
    Allocates memory to a destination buffer, copies from the cursor to the
    destination buffer using strncpy() and moves forward the cursor.

    See comments on alloc_and_memcpy() for more details.

    @param[out] destination the destination buffer.
    @param[in] length the amount of bytes to allocate and read from the buffer
                      (and to forward).
    @param[in] flags flags to pass to MySQL server my_malloc() function.
  */
  void alloc_and_strncpy(char **destination, size_t length, int flags);

  /**
    Reads string from cursor.

    Reads in the following format:
    1) Reads length stored on cursor first index. Moves cursor forward 1 byte.
    2) Set destination pointer to the cursor. Moves cursor forward length bytes.

    @param[out] destination the destination pointer.
    @param[out] length the amount of bytes to allocate and read from the buffer
                       (and to move forward).
  */
  void read_str_at_most_255_bytes(const char **destination, uint8_t *length);

  /**
    Reads a packed value.

    This function can move the cursor forward by 1, 3, 4 or 9 bytes depending on
    the value to be returned.

    @return the packed value.
  */
  uint64_t net_field_length_ll();

  /**
    Reads a transaction context data set.

    @param[in] set_len length of the set object (and to move forward).
    @param[out] set pointer to the set object to be filled.
  */
  void read_data_set(uint32_t set_len, std::list<const char *> *set);

  /**
    Reads a view change certification map.

    @param[in] map_len the length of the certification info map (and to move
                       forward).
    @param[out] map the certification info map to be filled.
  */
  void read_data_map(uint32_t map_len, std::map<std::string, std::string> *map);

  /**
    Copy a string into the destination buffer up to a max length.

    @param[out] destination the destination buffer.
    @param[in] max_length the max length to copy from the cursor.
    @param[in] dest_length the max length supported by the destination buffer.
  */
  void strncpyz(char *destination, size_t max_length, size_t dest_length);

  /**
    Fills a vector with a sequence of bytes from the cursor.

    @param[out] destination the vector be filled.
    @param[in] length the amount of bytes to read from the cursor (and to move
                      forward).
  */
  void assign(std::vector<uint8_t> *destination, size_t length);

 private:
  /* The buffer with the serialized binary log event */
  const char *m_buffer;
  /* The cursor: a pointer to the current read position in the buffer */
  const char *m_ptr;
  /* The length of the buffer */
  unsigned long long m_length;
  /* The limit the reader shall respect when reading from the buffer */
  unsigned long long m_limit;
  /* The pointer to the current error message, or nullptr */
  const char *m_error;

  /**
    Wrapper to le16toh to be used by read_and_letoh function.

    @param[in] value the value to be converted.

    @return the converted value.
  */
  uint16_t letoh(uint16_t value) { return le16toh(value); }

  /**
    Wrapper to le32toh to be used by read_and_letoh function.

    @param[in] value the value to be converted.

    @return the converted value.
  */
  int32_t letoh(int32_t value) { return le32toh(value); }

  /**
    Wrapper to le32toh to be used by read_and_letoh function.

    @param[in] value the value to be converted.

    @return the converted value.
  */
  uint32_t letoh(uint32_t value) { return le32toh(value); }

  /**
    Wrapper to le64toh to be used by read_and_letoh function.

    @param[in] value the value to be converted.

    @return the converted value.
  */
  int64_t letoh(int64_t value) { return le64toh(value); }

  /**
    Wrapper to le64toh to be used by read_and_letoh function.

    @param[in] value the value to be converted.

    @return the converted value.
  */
  uint64_t letoh(uint64_t value) { return le64toh(value); }
};
}  // end namespace binary_log
/**
  @} (end of group Replication)
*/
#endif /* EVENT_READER_INCLUDED */
