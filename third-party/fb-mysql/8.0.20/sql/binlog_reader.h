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

#ifndef BINLOG_READER_INCLUDED
#define BINLOG_READER_INCLUDED
#include "sql/binlog_istream.h"
#include "sql/log_event.h"

/**
   Deserialize a binlog event from event_data. event_data is serialized
   event object. It is a chunk of data in buffer.

   @param[in] event_data The event data used for deserialization.
   @param[in] event_data_len Length of event_data
   @param[in] fde The format_description_event of the event
   @param[in] verify_checksum  Verify event_data's checksum if it is true.
   @param[out] event  the event object generated.

   @retval Binlog_read_error::SUCCEED Succeed
   @retval Other than Binlog_read_error::SUCCEED Error
*/
Binlog_read_error::Error_type binlog_event_deserialize(
    const unsigned char *event_data, unsigned int event_data_len,
    const Format_description_event *fde, bool verify_checksum,
    Log_event **event);

class Default_binlog_event_allocator {
 public:
  enum { DELEGATE_MEMORY_TO_EVENT_OBJECT = true };
  unsigned char *allocate(size_t);
  void deallocate(unsigned char *ptr);
};
/**
   Binlog_event_data_istream fetches byte data from Basic_istream and
   divides them into event_data chunk according to the format. Event_data is a
   serialized event object. It is a chunk of data in buffer.
*/
class Binlog_event_data_istream {
 public:
  Binlog_event_data_istream(Binlog_read_error *error, Basic_istream *istream,
                            unsigned int max_event_size);
  Binlog_event_data_istream() = delete;
  Binlog_event_data_istream(const Binlog_event_data_istream &) = delete;
  Binlog_event_data_istream &operator=(const Binlog_event_data_istream &) =
      delete;
  virtual ~Binlog_event_data_istream() {}

  /**
     Read an event data from the stream and verify its checksum if
     verify_checksum is true.

     @param[out] data The pointer of the event data
     @param[out] length The length of the event data
     @param[in] allocator It is used to allocate memory for the event data.
     @param[in] verify_checksum Verify the event data's checksum if it is true.
     @param[in] checksum_alg Checksum algorithm for verifying the event data. It
                             is used only when verify_checksum is true.
     @retval false Succeed
     @retval true Error
  */
  template <class ALLOCATOR>
  bool read_event_data(unsigned char **data, unsigned int *length,
                       ALLOCATOR *allocator, bool verify_checksum,
                       enum_binlog_checksum_alg checksum_alg) {
    DBUG_TRACE;
    if (read_event_header() || check_event_header()) return true;

    unsigned char *event_data = allocator->allocate(m_event_length);
    if (event_data == nullptr)
      return m_error->set_type(Binlog_read_error::MEM_ALLOCATE);

    if (fill_event_data(event_data, verify_checksum, checksum_alg)) {
      allocator->deallocate(event_data);
      return true;
    }
    *data = event_data;
    *length = m_event_length;
    return false;
  }

 protected:
  unsigned char m_header[LOG_EVENT_MINIMAL_HEADER_LEN];
  /**
     Read the event header from the Basic_istream

     @retval false Succeed
     @retval true Error
  */
  virtual bool read_event_header();
  /**
     Check if it is a valid event header

     @retval false Succeed
     @retval true Error
  */
  bool check_event_header();
  /**
     Read fixed length of data from Basic_istream. It sets error to
     - Binlog_read_error::SYTEM_IO if Basic_istream returns error.
     - Binlog_read_error::TRUNC_EVENT if less than length is read.
     - ERROR_TYPE if zero byte is read. ERROR_TYPE is either TRUNC_EVENT or
       READ_EOF.

     @param[in] data    The buffer where the data stored.
     @param[in] length  Bytes of the data to be read.
     @retval false Succeed
     @retval true Error
  */
  template <Binlog_read_error::Error_type ERROR_TYPE>
  bool read_fixed_length(unsigned char *data, unsigned int length) {
    DBUG_TRACE;
    if (length == 0) return false;

    longlong ret = m_istream->read(data, length);
    if (ret == length) return false;
    switch (ret) {
      case -1:
        return m_error->set_type(Binlog_read_error::SYSTEM_IO);
      case 0:
        return m_error->set_type(ERROR_TYPE);
      default:
        return m_error->set_type(Binlog_read_error::TRUNC_EVENT);
    }
  }

  /**
     It is convenient for caller to share a Binlog_read_error object between
     streams. So Binlog_read_error pointer is defined here. It should be
     initialized in constructor by caller.
  */
  Binlog_read_error *m_error;

 private:
  Basic_istream *m_istream = nullptr;
  unsigned int m_max_event_size;
  unsigned int m_event_length = 0;

  /**
     Fill the event data into the given buffer and verify checksum if
     'verify_checksum' is true.

     @param[in] event_data The buffer where the event data will be stored.
     @param[in] verify_checksum Verify the event data's checksum if it is true.
     @param[in] checksum_alg Checksum algorithm for verifying the event data. It
                             is used only when verify_checksum is true.
     @retval false Succeed
     @retval true Error
  */
  bool fill_event_data(unsigned char *event_data, bool verify_checksum,
                       enum_binlog_checksum_alg checksum_alg);
};

/**
   It reads event_data from an event_data stream and deserialize them to event
   object.
*/
template <class EVENT_DATA_ISTREAM>
class Binlog_event_object_istream {
 public:
  Binlog_event_object_istream(Binlog_read_error *error,
                              EVENT_DATA_ISTREAM *istream)
      : m_error(error), m_data_istream(istream) {}

  Binlog_event_object_istream() = delete;
  Binlog_event_object_istream(const Binlog_event_object_istream &) = delete;
  Binlog_event_object_istream &operator=(const Binlog_event_object_istream &) =
      delete;

  /**
     Read an event ojbect from the stream

     @param[in] fde The Format_description_event for deserialization.
     @param[in] verify_checksum Verify the checksum of the event_data before
     @param[in] allocator It is used to allocate memory for the event data.
     @return An valid event object if succeed.
     @retval nullptr Error
  */
  template <class ALLOCATOR>
  Log_event *read_event_object(const Format_description_event &fde,
                               bool verify_checksum, ALLOCATOR *allocator,
                               unsigned int *read_length = nullptr) {
    DBUG_TRACE;
    unsigned char *data = nullptr;
    unsigned int length = 0;

    if (m_data_istream->read_event_data(&data, &length, allocator, false,
                                        fde.footer()->checksum_alg))
      return nullptr;

    Log_event *event = nullptr;
    if (m_error->set_type(binlog_event_deserialize(data, length, &fde,
                                                   verify_checksum, &event))) {
      allocator->deallocate(data);
      return nullptr;
    }

    event->register_temp_buf(reinterpret_cast<char *>(data),
                             ALLOCATOR::DELEGATE_MEMORY_TO_EVENT_OBJECT);
    if (read_length) *read_length = length;

    return event;
  }

 private:
  /**
     It is convenient for caller to share a Binlog_read_error object between
     streams. So Binlog_read_error pointer is defined here. It should be
     initialized in constructor by caller.
  */
  Binlog_read_error *m_error;
  EVENT_DATA_ISTREAM *m_data_istream = nullptr;
};

/**
   It owns an allocator, a byte stream, an event_data stream and an event object
   stream. The stream pipeline is setup in the constructor. All the objects
   required for reading a binlog file is initialized in reader class.

   It maintains the Format_description_event which is needed for reading the
   following binlog events. A default format_description_event is initialized
   at the begining. Then it will be replaced by the one read from the binlog
   file.

   Some convenient functions is added to encapsulate the access of IFILE,
   EVENT_DATA_ISTREAM, EVENT_OBJECT_ISTREAM. It makes the code
   simpler for reading a binlog file.
*/
template <class IFILE, class EVENT_DATA_ISTREAM,
          template <class> class EVENT_OBJECT_ISTREAM, class ALLOCATOR>
class Basic_binlog_file_reader {
 public:
  typedef EVENT_DATA_ISTREAM Event_data_istream;
  typedef EVENT_OBJECT_ISTREAM<Event_data_istream> Event_object_istream;

  Basic_binlog_file_reader(bool verify_checksum,
                           unsigned int max_event_size = UINT_MAX)
      : m_ifile(&m_error),
        m_data_istream(&m_error, &m_ifile, max_event_size),
        m_object_istream(&m_error, &m_data_istream),
        m_fde(BINLOG_VERSION, ::server_version),
        m_verify_checksum(verify_checksum) {}

  Basic_binlog_file_reader(const Basic_binlog_file_reader &) = delete;
  Basic_binlog_file_reader &operator=(const Basic_binlog_file_reader &) =
      delete;
  ~Basic_binlog_file_reader() { close(); }

  /**
     Open a binlog file and set read position to offset. It will read and store
     Format_description_event automatically if offset is bigger than current
     position and fde is nullptr. Otherwise fde is use instead of finding fde
     from the file if fde is not null.

     @param[in] file_name  name of the binlog file which will be opened.
     @param[in] offset  The position where it starts to read.
     @param[out] fdle   For returning an Format_description_log_event object.
     @retval false Succeed
     @retval true Error
  */
  bool open(const char *file_name, my_off_t offset = 0,
            Format_description_log_event **fdle = nullptr) {
    DBUG_TRACE;
    if (m_ifile.open(file_name)) return true;

    Format_description_log_event *fd = read_fdle(offset);
    if (!fd) return has_fatal_error();

    if (position() < offset && seek(offset)) {
      delete fd;
      return true;
    }
    if (fdle)
      *fdle = fd;
    else
      delete fd;
    return false;
  }
  /**
     Close the binlog file.
  */
  void close() {
    m_ifile.close();
    m_fde = Format_description_event(BINLOG_VERSION, server_version);
  }

  bool is_open() { return m_ifile.is_open(); }
  my_off_t position() { return m_ifile.position(); }
  bool seek(my_off_t pos) { return m_ifile.seek(pos); }

  /**
     Wrapper of EVENT_DATA_ISTREAM::read_event_data.
  */
  bool read_event_data(unsigned char **data, unsigned int *length) {
    m_event_start_pos = position();
    return m_data_istream.read_event_data(data, length, &m_allocator,
                                          m_verify_checksum,
                                          m_fde.footer()->checksum_alg);
  }
  /**
     wrapper of EVENT_OBJECT_ISTREAM::read_event_object.
  */
  Log_event *read_event_object(unsigned int *read_length = nullptr) {
    m_event_start_pos = position();
    Log_event *ev = m_object_istream.read_event_object(
        m_fde, m_verify_checksum, &m_allocator, read_length);
    if (ev && ev->get_type_code() == binary_log::FORMAT_DESCRIPTION_EVENT)
      m_fde = dynamic_cast<Format_description_event &>(*ev);
    return ev;
  }

  bool has_fatal_error() { return m_error.has_fatal_error(); }
  /**
     Return the error happened in the stream pipeline.
  */
  Binlog_read_error::Error_type get_error_type() { return m_error.get_type(); }
  /**
     Return the error message of the error happened in the stream pipeline.
  */
  const char *get_error_str() { return m_error.get_str(); }

  IFILE *ifile() { return &m_ifile; }
  Event_data_istream *event_data_istream() { return &m_data_istream; }
  Event_object_istream *event_object_istream() { return &m_object_istream; }
  ALLOCATOR *allocator() { return &m_allocator; }

  void set_format_description_event(const Format_description_event &fde) {
    m_fde = fde;
  }
  const Format_description_event *format_description_event() { return &m_fde; }
  my_off_t event_start_pos() { return m_event_start_pos; }

 private:
  Binlog_read_error m_error;

  IFILE m_ifile;
  Event_data_istream m_data_istream;
  Event_object_istream m_object_istream;
  ALLOCATOR m_allocator;

  Format_description_event m_fde;
  bool m_verify_checksum = false;
  my_off_t m_event_start_pos = 0;

  /**
     Read the Format_description_log_events before 'offset'.

     @param[in] offset The position where the read should stop.
     @return A valid Format_description_log_event pointer or nullptr.
  */
  Format_description_log_event *read_fdle(my_off_t offset) {
    DBUG_TRACE;
    Default_binlog_event_allocator allocator;
    Format_description_log_event *fdle = nullptr;
    /*
      Format_description_event is skipped, so we initialize m_fde here. For
      relay log, it need to find master's Format_description_event. master's
      Format_description_event is the 3rd or 4th event of a relay log file.
      Relay log's format looks like:
      Format_description_event : relay log's Format_description_event
      Previous_gtid_event
      [Rotate_event]           : In the case rotating relaylog, no Rotate here
      Format_description_event : master's Format_description_event
    */
    while (position() < offset) {
      m_event_start_pos = position();

      Log_event *ev = m_object_istream.read_event_object(
          m_fde, m_verify_checksum, &allocator);

      if (ev == nullptr) break;
      if (ev->get_type_code() == binary_log::FORMAT_DESCRIPTION_EVENT) {
        delete fdle;
        fdle = dynamic_cast<Format_description_log_event *>(ev);
        m_fde = *fdle;
      } else {
        binary_log::Log_event_type type = ev->get_type_code();
        delete ev;
        if (type != binary_log::PREVIOUS_GTIDS_LOG_EVENT &&
            type != binary_log::ROTATE_EVENT)
          break;
      }
    }
    if (has_fatal_error()) {
      delete fdle;
      return nullptr;
    }
    return fdle;
  }
};

#ifdef MYSQL_SERVER
typedef Basic_binlog_file_reader<Binlog_ifile, Binlog_event_data_istream,
                                 Binlog_event_object_istream,
                                 Default_binlog_event_allocator>
    Binlog_file_reader;
typedef Basic_binlog_file_reader<Relaylog_ifile, Binlog_event_data_istream,
                                 Binlog_event_object_istream,
                                 Default_binlog_event_allocator>
    Relaylog_file_reader;
#endif  // MYSQL_SERVER
#endif  // BINLOG_READER_INCLUDED
