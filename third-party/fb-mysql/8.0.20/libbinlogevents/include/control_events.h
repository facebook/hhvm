/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

  @file control_events.h

  @brief Contains the classes representing events operating in the replication
  stream properties. Each event is represented as a byte sequence with logical
  divisions as event header, event specific data and event footer. The header
  and footer are common to all the events and are represented as two different
  subclasses.
*/

#ifndef CONTROL_EVENT_INCLUDED
#define CONTROL_EVENT_INCLUDED

#include <sys/types.h>
#include <time.h>
#include <list>
#include <map>
#include <array>
#include <vector>

#include "binlog_event.h"
#include "template_utils.h"
#include "uuid.h"

#include "compression/base.h"

namespace binary_log {
/**
  @class Rotate_event

  When a binary log file exceeds a size limit, a ROTATE_EVENT is written
  at the end of the file that points to the next file in the squence.
  This event is information for the slave to know the name of the next
  binary log it is going to receive.

  ROTATE_EVENT is generated locally and written to the binary log
  on the master. It is written to the relay log on the slave when FLUSH LOGS
  occurs, and when receiving a ROTATE_EVENT from the master.
  In the latter case, there will be two rotate events in total originating
  on different servers.

  @section Rotate_event_binary_format Binary Format

  <table>
  <caption>Post-Header for Rotate_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>position</td>
    <td>8 byte integer</td>
    <td>The position within the binary log to rotate to.</td>
  </tr>

  </table>

  The Body has one component:

  <table>
  <caption>Body for Rotate_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>new_log_ident</td>
    <td>variable length string without trailing zero, extending to the
    end of the event (determined by the length field of the
    Common-Header)
    </td>
    <td>Name of the binlog to rotate to.</td>
  </tr>

  </table>
*/
class Rotate_event : public Binary_log_event {
 public:
  const char *new_log_ident;
  size_t ident_len;
  unsigned int flags;
  uint64_t pos;

  enum {
    /* Values taken by the flag member variable */
    DUP_NAME = 2,  // if constructor should dup the string argument
    RELAY_LOG = 4  // rotate event for the relay log
  };

  enum {
    /* Rotate event post_header */
    R_POS_OFFSET = 0,
    R_IDENT_OFFSET = 8
  };

  /**
    This is the minimal constructor, it will set the type code as ROTATE_EVENT.
  */
  Rotate_event(const char *new_log_ident_arg, size_t ident_len_arg,
               unsigned int flags_arg, uint64_t pos_arg)
      : Binary_log_event(ROTATE_EVENT),
        new_log_ident(new_log_ident_arg),
        ident_len(ident_len_arg ? ident_len_arg : strlen(new_log_ident_arg)),
        flags(flags_arg),
        pos(pos_arg) {}

  /**
    The layout of Rotate_event data part is as follows:

    <pre>
    +-----------------------------------------------------------------------+
    | common_header | post_header | position of the first event | file name |
    +-----------------------------------------------------------------------+
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event, used to get the following information:
                  -binlog_version
                  -server_version
                  -post_header_len
                  -common_header_len
                The content of this object depends on the binlog-version
                currently in use.
  */
  Rotate_event(const char *buf, const Format_description_event *fde);

#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &);
  void print_long_info(std::ostream &);
#endif

  ~Rotate_event() {
    if (flags & DUP_NAME) bapi_free(const_cast<char *>(new_log_ident));
  }
};

/**
  @class Format_description_event
  For binlog version 4.
  This event is saved by threads which read it, as they need it for future
  use (to decode the ordinary events).

  @section Format_description_event_binary_format Binary Format

  The Post-Header has six components:

  <table>
  <caption>Post-Header for Format_description_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>created</td>
    <td>4 byte unsigned integer</td>
    <td>The creation timestamp, if non-zero,
        is the time in seconds when this event was created</td>
  </tr>
  <tr>
    <td>binlog_version</td>
    <td>2 byte unsigned integer</td>
    <td>This is 1 in MySQL 3.23 and 3 in MySQL 4.0 and 4.1
        (In MySQL 5.0 and up, FORMAT_DESCRIPTION_EVENT is
        used instead of START_EVENT_V3 and for them its 4).</td>
  </tr>
  <tr>
    <td>server_version</td>
    <td>char array of 50 bytes</td>
    <td>The MySQL server's version (example: 4.0.14-debug-log),
        padded with 0x00 bytes on the right</td>
  </tr>
  <tr>
    <td>common_header_len</td>
    <td>1 byte unsigned integer</td>
    <td>The length of the event header. This value includes the extra_headers
        field, so this header length - 19 yields the size
        of the extra_headers field.</td>
  </tr>
  <tr>
    <td>post_header_len</td>
    <td>array of type 1 byte unsigned integer</td>
    <td>The lengths for the fixed data part of each event</td>
  </tr>
  <tr>
    <td>server_version_split</td>
    <td>unsigned char array</td>
    <td>Stores the server version of the server
        and splits them in three parts</td>
  </tr>
  <tr>
    <td>number_of_event_types</td>
    <td>1 byte unsigned integer</td>
    <td>number of event types present in the server</td>
  </tr>
  </table>
*/
class Format_description_event : public Binary_log_event {
 public:
  /**
     If this event is at the start of the first binary log since server
     startup 'created' should be the timestamp when the event (and the
     binary log) was created.  In the other case (i.e. this event is at
     the start of a binary log created by FLUSH LOGS or automatic
     rotation), 'created' should be 0.  This "trick" is used by MySQL
     >=4.0.14 slaves to know whether they must drop stale temporary
     tables and whether they should abort unfinished transaction.

     Note that when 'created'!=0, it is always equal to the event's
     timestamp; indeed Format_description_event is written only in binlog.cc
     where the first constructor below is called, in which 'created' is set to
     'when'.  So in fact 'created' is a useless variable. When it is 0 we can
     read the actual value from timestamp ('when') and when it is non-zero we
     can read the same value from timestamp
     ('when'). Conclusion:
     - we use timestamp to print when the binlog was created.
     - we use 'created' only to know if this is a first binlog or not.
  */
  time_t created;
  uint16_t binlog_version;
  char server_version[ST_SERVER_VER_LEN];
  /*
    We set this to 1 if we don't want to have the created time in the log,
    which is the case when we rollover to a new log.
  */
  bool dont_set_created;

  /**
     The size of the fixed header which _all_ events have
     (for binlogs written by this version, this is equal to
     LOG_EVENT_HEADER_LEN), except FORMAT_DESCRIPTION_EVENT and ROTATE_EVENT
     (those have a header of size LOG_EVENT_MINIMAL_HEADER_LEN).
  */
  uint8_t common_header_len;
  /*
    The list of post-headers' lengths followed
    by the checksum alg decription byte
  */
  std::vector<uint8_t> post_header_len;
  unsigned char server_version_split[ST_SERVER_VER_SPLIT_LEN];

  /**
     Format_description_event 1st constructor.

     This constructor can be used to create the event to write to the binary log
     (when the server starts or when FLUSH LOGS)

     @param binlog_ver             the binlog version for which we want to build
     an event. It should only be 4, old versions are not compatible anymore
     since 8.0.2.
     @param server_ver             The MySQL server's version.
  */
  Format_description_event(uint8_t binlog_ver, const char *server_ver);
  /**
    The layout of Format_description_event data part is as follows:

    <pre>
    +=====================================+
    | event  | binlog_version   19 : 2    | = 4
    | data   +----------------------------+
    |        | server_version   21 : 50   |
    |        +----------------------------+
    |        | create_timestamp 71 : 4    |
    |        +----------------------------+
    |        | header_length    75 : 1    |
    |        +----------------------------+
    |        | post-header      76 : n    | = array of n bytes, one byte
    |        | lengths for all            |   per event type that the
    |        | event types                |   server knows about
    +=====================================+
    </pre>
    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).

    @note The fde passed to this constructor was created through another
          constructor of FDE class.
  */
  Format_description_event(const char *buf,
                           const Format_description_event *fde);

  Format_description_event(const Format_description_event &) = default;
  Format_description_event &operator=(const Format_description_event &) =
      default;
  uint8_t number_of_event_types;
  /**
    This method is used to find out the version of server that originated
    the current FD instance.

    @return the version of server.
  */
  unsigned long get_product_version() const;
  /**
    This method checks the MySQL version to determine whether checksums may be
    present in the events contained in the binary log.

    @retval true  if the event's version is earlier than one that introduced
                  the replication event checksum.
    @retval false otherwise.
  */
  bool is_version_before_checksum() const;
  /**
    This method populates the array server_version_split which is then used for
    lookups to find if the server which created this event has some known bug.
  */
  void calc_server_version_split();
#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &info);
  void print_long_info(std::ostream &info);
#endif
  ~Format_description_event();

  bool header_is_valid() const {
    return ((common_header_len >= LOG_EVENT_MINIMAL_HEADER_LEN) &&
            (!post_header_len.empty()));
  }

  bool version_is_valid() const {
    /* It is invalid only when all version numbers are 0 */
    return server_version_split[0] != 0 || server_version_split[1] != 0 ||
           server_version_split[2] != 0;
  }
};

/**
  @class Stop_event

  A stop event is written to the log files under these circumstances:
  - A master writes the event to the binary log when it shuts down.
  - A slave writes the event to the relay log when it shuts down or
    when a RESET SLAVE statement is executed.

  @section Stop_event_binary_format Binary Format

  The Post-Header and Body for this event type are empty; it only has
  the Common-Header.
*/

class Stop_event : public Binary_log_event {
 public:
  /**
    It is the minimal constructor, and all it will do is set the type_code as
    STOP_EVENT in the header object in Binary_log_event.
  */
  Stop_event() : Binary_log_event(STOP_EVENT) {}

  /**
    A Stop_event is occurs under these circumstances:
    -  A master writes the event to the binary log when it shuts down
    -  A slave writes the event to the relay log when it shuts down or when a
       RESET SLAVE statement is executed
    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Stop_event(const char *buf, const Format_description_event *fde);

#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &info);
#endif
};

/**
  @class Incident_event

   Class representing an incident, an occurance out of the ordinary,
   that happened on the master.

   The event is used to inform the slave that something out of the
   ordinary happened on the master that might cause the database to be
   in an inconsistent state.

  @section Incident_event_binary_format Binary Format

   <table id="IncidentFormat">
   <caption>Incident event format</caption>
   <tr>
     <th>Symbol</th>
     <th>Format</th>
     <th>Description</th>
   </tr>
   <tr>
     <td>INCIDENT</td>
     <td align="right">2</td>
     <td>Incident number as an unsigned integer</td>
   </tr>
   <tr>
     <td>MSGLEN</td>
     <td align="right">1</td>
     <td>Message length as an unsigned integer</td>
   </tr>
   <tr>
     <td>MESSAGE</td>
     <td align="right">MSGLEN</td>
     <td>The message, if present. Not null terminated.</td>
   </tr>
   </table>

*/
class Incident_event : public Binary_log_event {
 public:
  /**
    Enumeration of the incidents that can occur for the server.
  */
  enum enum_incident {
    /** No incident */
    INCIDENT_NONE = 0,
    /** There are possibly lost events in the replication stream */
    INCIDENT_LOST_EVENTS = 1,
    /** Shall be last event of the enumeration */
    INCIDENT_COUNT
  };

  enum_incident get_incident_type() { return incident; }
  char *get_message() { return message; }

  /**
    This will create an Incident_event with an empty message and set the
    type_code as INCIDENT_EVENT in the header object in Binary_log_event.
  */
  explicit Incident_event(enum_incident incident_arg)
      : Binary_log_event(INCIDENT_EVENT),
        incident(incident_arg),
        message(nullptr),
        message_length(0) {}

  /**
    Constructor of Incident_event
    The buffer layout is as follows:
    <pre>
    +-----------------------------------------------------+
    | Incident_number | message_length | Incident_message |
    +-----------------------------------------------------+
    </pre>

    Incident number codes are listed in binlog_evnet.h.
    The only code currently used is INCIDENT_LOST_EVENTS, which indicates that
    there may be lost events (a "gap") in the replication stream that requires
    databases to be resynchronized.

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Incident_event(const char *buf, const Format_description_event *fde);
#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &info);
  void print_long_info(std::ostream &info);
#endif
 protected:
  enum_incident incident;
  char *message;
  size_t message_length;
};

/**
  @class Xid_event

  An XID event is generated for a commit of a transaction that modifies one or
  more tables of an XA-capable storage engine.

  @section Xid_event_binary_format Binary Format

The Body has the following component:

  <table>
  <caption>Body for Xid_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>xid</td>
    <td>8 byte unsigned integer</td>
    <td>The XID transaction number.</td>
  </tr>
  </table>
  The Post-Header and Body for this event type are empty; it only has
  the common header.
*/
class Xid_event : public Binary_log_event {
 public:
  /**
    The minimal constructor of Xid_event, it initializes the instance variable
    xid and set the type_code as XID_EVENT in the header object in
    Binary_log_event
  */
  explicit Xid_event(uint64_t xid_arg)
      : Binary_log_event(XID_EVENT), xid(xid_arg) {}

  /**
    An XID event is generated for a commit of a transaction that modifies one or
    more tables of an XA-capable storage engine
    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Xid_event(const char *buf, const Format_description_event *fde);
  uint64_t xid;
#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &info);
  void print_long_info(std::ostream &info);
#endif
};

/**
  @class XA_prepare_event

  An XA_prepare event is generated for a XA prepared transaction.
  Like Xid_event it contans XID of the *prepared* transaction.

  @section XA_prepare_event_binary_format Binary Format

The Body has the following component:

  <table>
  <caption>Body for XA_prepare_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>my_xid</td>
    <td>a struct similar to mysql/plugin.h containing three members.</td>
    <td>serialized XID representation of XA transaction.</td>
  </tr>

  <tr>
    <td>xid</td>
    <td>a pointer to XID object.</td>
    <td>a reference to an object for mysql logger.</td>
  </tr>

  <tr>
    <td>one_phase</td>
    <td>a bool</td>
    <td>the value specifies the current XA transaction commit method.</td>
  </tr>
  </table>
  The Post-Header and Body for this event type are empty; it only has
  the common header.
*/

class XA_prepare_event : public Binary_log_event {
  /*
    Struct def is copied from $MYSQL/include/mysql/plugin.h,
    consult there about fine details.
  */
  static const int MY_XIDDATASIZE = 128;

  struct MY_XID {
    long formatID;
    long gtrid_length;
    long bqual_length;
    char data[MY_XIDDATASIZE]; /* Not \0-terminated */
  };

 protected:
  /* size of serialization buffer is explained in $MYSQL/sql/xa.h. */
  static const uint16_t ser_buf_size =
      8 + 2 * MY_XIDDATASIZE + 4 * sizeof(long) + 1;
  MY_XID my_xid;
  void *xid; /* Master side only */
  bool one_phase;

 public:
  /**
    The minimal constructor of XA_prepare_event, it initializes the
    instance variable xid and set the type_code as XID_EVENT in the
    header object in Binary_log_event
  */
  XA_prepare_event(void *xid_arg, bool oph_arg)
      : Binary_log_event(XA_PREPARE_LOG_EVENT),
        xid(xid_arg),
        one_phase(oph_arg) {}

  /**
    An XID event is generated for a commit of a transaction that modifies one or
    more tables of an XA-capable storage engine
    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  XA_prepare_event(const char *buf, const Format_description_event *fde);
#ifndef HAVE_MYSYS
  /*
    todo: we need to find way how to exploit server's code of
    serialize_xid()
  */
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif
};

/**
  @class Ignorable_event

  Base class for ignorable log events. Events deriving from
  this class can be safely ignored by slaves that cannot
  recognize them. Newer slaves, will be able to read and
  handle them. This has been designed to be an open-ended
  architecture, so adding new derived events shall not harm
  the old slaves that support ignorable log event mechanism
  (they will just ignore unrecognized ignorable events).

  @note The only thing that makes an event ignorable is that it has
  the LOG_EVENT_IGNORABLE_F flag set.  It is not strictly necessary
  that ignorable event types derive from Ignorable_event; they may
  just as well derive from Binary_log_event and Log_event and pass
  LOG_EVENT_IGNORABLE_F as argument to the Log_event constructor.

  @section Ignoarble_event_binary_format Binary format

  The Post-Header and Body for this event type are empty; it only has
  the Common-Header.
*/
class Ignorable_event : public Binary_log_event {
 public:
  // buf is advanced in Binary_log_event constructor to point to beginning of
  // post-header

  /**
    The minimal constructor and all it will do is set the type_code as
    IGNORABLE_LOG_EVENT in the header object in Binary_log_event.
  */
  explicit Ignorable_event(Log_event_type type_arg = IGNORABLE_LOG_EVENT)
      : Binary_log_event(type_arg) {}
  /**
    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Ignorable_event(const char *buf, const Format_description_event *fde);
#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif
};

class Metadata_event : public Binary_log_event {
 public:
  /**
   * Create a new Metadata event object
   */
  Metadata_event() : Binary_log_event(METADATA_EVENT) {}

  /**
   * Create a new metadata event by deserializing buffer
   *
   * @param buf - The buffer to deserialize from
   * @param fde - Format description to deserialize the event
   */
  Metadata_event(const char *buf, const Format_description_event *fde);

#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif

  /**
   * Set hlc_time and update internal state needed later to write this to
   * stream
   *
   * @param hlc_time_ns - HLC timestamp to set
   */
  void set_hlc_time(uint64_t hlc_time_ns);

  /**
   * Get hlc_time
   *
   * @return hlc_time_ns if present. 0 otherwise
   */
  uint64_t get_hlc_time() const;

  /**
   * Set prev_hlc_time and update internal state needed later to write this to
   * stream
   *
   * @param prev_hlc_time_ns - Previous HLC timestamp to set
   */
  void set_prev_hlc_time(uint64_t prev_hlc_time_ns);

  /**
   * Get prev_hlc_time
   *
   * @return prev_hlc_time_ns if present. 0 otherwise
   */
  uint64_t get_prev_hlc_time() const;

  /**
   * Get raft term
   *
   * @return raft_term if present. -1 otherwise
   */
  int64_t get_raft_term() const;

  /**
   * Get raft_index
   *
   * @return raft_index if present. -1 otherwise
   */
  int64_t get_raft_index() const;

  /**
   * Set raft_term and raft_index and update internal state needed later to
   * write this to stream
   *
   * @param term - Raft term to sert
   * @param index - Raft index to sert
   */
  void set_raft_term_and_index(int64_t term, int64_t index);

  /**
   * Adds a raft string field type into the metadata log event.
   *
   * @param str - the raft provided string
   */
  void set_raft_str(const std::string &str);

  /**
   * Get the generic string field reference.
   *
   * @return raft_str if present, null string if not.
   */
  const std::string &get_raft_str() const;

  /**
   * Set previous file's last raft_term and raft_index, i.e.
   * the opid of the rotate event to the metadata event.
   *
   * @param term - Raft term to set
   * @param index - Raft index to set
   */
  void set_raft_prev_opid(int64_t term, int64_t index);

  enum RAFT_ROTATE_EVENT_TAG : int16_t {
    RRET_SIMPLE_ROTATE = 0,  // Tag for a simple rotate event
    RRET_NOOP = 1,           // Tag for a NO-OP event
    RRET_CONFIG_CHANGE = 2,  // Tag for a config change event
    // If new types are added make sure all assertions
    // are checked for RRET_NOT_ROTATE
    RRET_NOT_ROTATE = 999,  // Not a rotate event or invalid tag
  };

  /**
   * Tag the rotate event before the metadata event with the
   * appropriate tag
   */
  void set_raft_rotate_tag(RAFT_ROTATE_EVENT_TAG t);

  /**
   * Get the rotate event tag.
   */
  RAFT_ROTATE_EVENT_TAG get_rotate_tag() const;

  /**
   * Get the rotate tag as a human readable string.
   */
  std::string get_rotate_tag_string() const;

  /**
   * Get raft previous opid term
   *
   * @return prev_raft_term if present. -1 otherwise
   */
  int64_t get_raft_prev_opid_term() const;

  /**
   * Get raft previous opid index
   *
   * @return prev_raft_index if present. -1 otherwise
   */
  int64_t get_raft_prev_opid_index() const;

  /**
   * The spec for different 'types' supported by this event
   */
  enum class Metadata_event_types : unsigned char {
    /* The type corresponding to HLC timestamp */
    HLC_TYPE = 0,
    /* The type corresponding to Previous HLC timestamp. This is written after
     * Previous_gtid_log_event in a new binlog file. Previous HLC is max HLC
     * that could have been potentially stored in all the previous binlog for
     * the instance. Server uses this to set its HLC to the right value when it
     * is starting and it protects the HLC clock from moving back in time
     * (during cases like a reset master followed by server restart) */
    PREV_HLC_TYPE = 1,
    /* Raft term and index added by raft consensus plugin */
    RAFT_TERM_INDEX_TYPE = 2,
    /* Config added by raft consensus plugin */
    RAFT_GENERIC_STR_TYPE = 3,
    /* Raft term and index for the last file*/
    RAFT_PREV_OPID_TYPE = 4,
    /* Raft Rotate Event Tag Type */
    RAFT_ROTATE_TAG_TYPE = 5,
    METADATA_EVENT_TYPE_MAX,
  };

  /**
   * @returns TRUE if 'type' exists in this event, false otherwise
   */
  bool does_exist(Metadata_event_types type) const;

 protected:
  /**
   * Read the specified 'type' from the buffer and update internal state
   *
   * @param type - Metadata_log_event_type to read from the buffer
   *
   * @returns - The size(in bytes) of the type read from buffer
   */
  uint read_type(Metadata_event_types type);

  /**
   * Set a specific type as existing in this event
   *
   * @param The type to set to true to indicate it exists in thie event
   */
  void set_exist(Metadata_event_types type);

  /* Use 1 byte to encode the type of the field */
  static const uint32_t ENCODED_TYPE_SIZE = 1;
  /* Use 2 byte to encode the length of the field's value */
  static const uint32_t ENCODED_LENGTH_SIZE = 2;

  /* HLC timestamp. The type corresponding to this is HLC_TYPE. */
  uint64_t hlc_time_ns_ = 0;
  static const uint32_t ENCODED_HLC_SIZE = sizeof(hlc_time_ns_);

  /* Prev HLC timestamp. The type corresponding to this is PREV_HLC_TYPE. */
  uint64_t prev_hlc_time_ns_ = 0;
  static const uint32_t ENCODED_PREV_HLC_SIZE = sizeof(prev_hlc_time_ns_);

  /* Raft (term, index). Written and interpreted by raft consensus plugin.
   * The type corresponding to this is RAFT_TERM_INDEX_TYPE */
  int64_t raft_term_ = -1;
  int64_t raft_index_ = -1;
  static const uint32_t ENCODED_RAFT_TERM_INDEX_SIZE =
      sizeof(raft_term_) + sizeof(raft_index_);

  /* Raft generic string to be added to a metadata event. Written and
   * interpreted by raft plugin or by kuduraft. Currently being used
   * to convey a config change operation, when the Metadata event
   * preceedes the RotateEvent.
   */
  std::string raft_str_;

  /* Previous (term, index). Provided and interpreted by raft consensus plugin.
   * Since rotate events are consensus sync point, prev term and prev index is
   * committed
   * The type corresponding to this is RAFT_PREV_OPID_TYPE */
  int64_t prev_raft_term_ = -1;
  int64_t prev_raft_index_ = -1;
  static const uint32_t ENCODED_RAFT_PREV_OPID_SIZE =
      sizeof(prev_raft_term_) + sizeof(prev_raft_index_);

  RAFT_ROTATE_EVENT_TAG raft_rotate_tag_ = RRET_NOT_ROTATE;
  // will write as uint16_t
  static const uint32_t ENCODED_RAFT_ROTATE_TAG_SIZE = sizeof(uint16_t);

  /* Total size of this event when encoded into the stream */
  size_t size_ = 0;

  /* Types that exist in this event */
  std::array<bool, static_cast<std::size_t>(
                       Metadata_event_types::METADATA_EVENT_TYPE_MAX)>
      existing_types_ = {};
};

/**
  @struct  gtid_info
  Structure to hold the members declared in the class Gtid_log_event those
  member are objects of classes defined in server(rpl_gtid.h). As we can not
  move all the classes defined there(in rpl_gtid.h) in libbinlogevents so this
  structure was created, to provide a way to map the decoded value in Gtid_event
  ctor and the class members defined in rpl_gtid.h, these classes are also the
  members of Gtid_log_event(subclass of this in server code)

  The structure contains the following components.
  <table>
  <caption>Structure gtid_info</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>
  <tr>
    <td>rpl_gtid_sidno</td>
    <td>4 bytes integer</td>
    <td>SIDNO (source ID number, first component of GTID)</td>
  </tr>
  <tr>
    <td>rpl_gtid_gno</td>
    <td>8 bytes integer</td>
    <td>GNO (group number, second component of GTID)</td>
  </tr>
  </table>
*/
struct gtid_info {
  int32_t rpl_gtid_sidno;
  int64_t rpl_gtid_gno;
};

/**
  This event is a wrapper event and encloses many other events.

  It is mostly used for carrying compressed payloads as its content
  can be compressed, in which case, its metadata shall contain
  information about the compression metadata as well.
 */
class Transaction_payload_event : public Binary_log_event {
 private:
  Transaction_payload_event &operator=(const Transaction_payload_event &) =
      delete;
  Transaction_payload_event(const Transaction_payload_event &) = delete;

 public:
  /**
    The on-the-wire fields
   */
  enum fields {
    /** Marks the end of the payload header. */
    OTW_PAYLOAD_HEADER_END_MARK = 0,

    /** The payload field */
    OTW_PAYLOAD_SIZE_FIELD = 1,

    /** The compression type field */
    OTW_PAYLOAD_COMPRESSION_TYPE_FIELD = 2,

    /** The uncompressed size field */
    OTW_PAYLOAD_UNCOMPRESSED_SIZE_FIELD = 3,

    /** Other fields are appended here. */
  };

 protected:
  /**
    The raw bytes which are the data that this event contains.
   */
  const char *m_payload{nullptr};

  /**
    The size of the data.
   */
  uint64_t m_payload_size{0};

  /**
    If the data is compressed, which compression was used.

    For now, the only compressors supported are: ZSTD or NONE.

    NONE means no compression at all. ZSTD means using ZSTD compression.
   */
  transaction::compression::type m_compression_type{
      transaction::compression::type::NONE};

  /**
    The size of the data uncompressed. This is the same as @c m_payload_size if
    there is no compression involved.
   */
  uint64_t m_uncompressed_size{0};

 public:
  static const unsigned short COMPRESSION_TYPE_MIN_LENGTH = 1;
  static const unsigned short COMPRESSION_TYPE_MAX_LENGTH = 9;
  static const unsigned short PAYLOAD_SIZE_MIN_LENGTH = 0;
  static const unsigned short PAYLOAD_SIZE_MAX_LENGTH = 9;
  static const unsigned short UNCOMPRESSED_SIZE_MIN_LENGTH = 0;
  static const unsigned short UNCOMPRESSED_SIZE_MAX_LENGTH = 9;

  static const int MAX_DATA_LENGTH = COMPRESSION_TYPE_MAX_LENGTH +
                                     PAYLOAD_SIZE_MAX_LENGTH +
                                     UNCOMPRESSED_SIZE_MAX_LENGTH;

  /**
    Creates @c Transaction_payload_event with the given data which has the
    given size.

    @param payload the data that this event shall wrap.
    @param payload_size the size of the payload.

    The data shall not be compressed. However, there is no other validation
    that this is the case.
   */
  Transaction_payload_event(const char *payload, uint64_t payload_size);

  /**
    Creates @c Transaction_payload_event with the given data which has the
    given size. The data provided may or may not have been compressed. In
    any case the compression_type must be set.

    @param payload the data that this event shall wrap.
    @param payload_size the size of the payload.
    @param compression_type the compression type used for the data provided.
    @param uncompressed_size the size of the data when uncompressed.

    The data may or may not be compressed. There is no validation or check
    that it is or that the payload matches the metadata provided.
   */
  Transaction_payload_event(const char *payload, uint64_t payload_size,
                            uint16_t compression_type,
                            uint64_t uncompressed_size);

  /**
    This constructor takes a raw buffer and a format descriptor event and
    decodes the buffer. It populates this event metadata with the contents
    of the buffer.

    @param buf the buffer to decode.
    @param fde the format description event used to decode the buffer.
   */
  Transaction_payload_event(const char *buf,
                            const Format_description_event *fde);

  /**
    This destroys the transaction payload event.
   */
  virtual ~Transaction_payload_event() override;

  /**
    Shall set the compression type used for the enclosed payload.

    @param type the compression type.
   */
  void set_compression_type(transaction::compression::type type) {
    m_compression_type = type;
  }

  /**
    Shall return the compression type used for the enclosed payload.

    @return the compression type.
   */
  transaction::compression::type get_compression_type() const {
    return m_compression_type;
  }

  /**
    Shall set the size of the payload inside this event.

    @param size The payload size.
   */
  void set_payload_size(uint64_t size) { m_payload_size = size; }

  /**
    Shall get the size of the payload inside this event.

    @return The payload size.
   */
  uint64_t get_payload_size() const { return m_payload_size; }

  /**
    Shall set the uncompressed size of the payload.

    @param size the uncompressed size of the payload.
   */
  void set_uncompressed_size(uint64_t size) { m_uncompressed_size = size; }

  /**
    Shall get the uncompressed size of the event.

    @return uncompressed_size.
   */
  uint64_t get_uncompressed_size() const { return m_uncompressed_size; }

  /**
    Shall set the payload of the event.

    @param data the payload of the event.
   */
  void set_payload(const char *data) { m_payload = data; }

  /**
    Shall get the payload of the event.

    @return the payload of the event.
   */
  const char *get_payload() const { return m_payload; }

  /**
    Shall return a textual representation of this event.

    @return a textial representation of this event.
   */
  std::string to_string() const;

#ifndef HAVE_MYSYS
  virtual void print_event_info(std::ostream &) override;
  virtual void print_long_info(std::ostream &) override;
#endif
};

/**
  @class Gtid_event
  GTID stands for Global Transaction IDentifier
  It is composed of two parts:
    - SID for Source Identifier, and
    - GNO for Group Number.
  The basic idea is to
     -  Associate an identifier, the Global Transaction IDentifier or GTID,
        to every transaction.
     -  When a transaction is copied to a slave, re-executed on the slave,
        and written to the slave's binary log, the GTID is preserved.
     -  When a  slave connects to a master, the slave uses GTIDs instead of
        (file, offset)

  @section Gtid_event_binary_format Binary Format

  The Body can have up to nine components:

  <table>
  <caption>Body for Gtid_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>GTID_FLAGS</td>
    <td>1 byte</td>
    <td>00000001 = Transaction may have changes logged with SBR.
        In 5.6, 5.7.0-5.7.18, and 8.0.0-8.0.1, this flag is always set.
        Starting in 5.7.19 and 8.0.2, this flag is cleared if the transaction
        only contains row events. It is set if any part of the transaction is
        written in statement format.</td>
  </tr>
  <tr>
    <td>SID</td>
    <td>16 byte sequence</td>
    <td>UUID representing the SID</td>
  </tr>
  <tr>
    <td>GNO</td>
    <td>8 byte integer</td>
    <td>Group number, second component of GTID.</td>
  </tr>
  <tr>
    <td>logical clock timestamp typecode</td>
    <td>1 byte integer</td>
    <td>The type of logical timestamp used in the logical clock fields.</td>
  </tr>
  <tr>
    <td>last_committed</td>
    <td>8 byte integer</td>
    <td>Store the transaction's commit parent sequence_number</td>
  </tr>
  <tr>
    <td>sequence_number</td>
    <td>8 byte integer</td>
    <td>The transaction's logical timestamp assigned at prepare phase</td>
  </tr>
  <tr>
    <td>immediate_commit_timestamp</td>
    <td>7 byte integer</td>
    <td>Timestamp of commit on the immediate master</td>
  </tr>
  <tr>
    <td>original_commit_timestamp</td>
    <td>7 byte integer</td>
    <td>Timestamp of commit on the originating master</td>
  </tr>
  <tr>
    <td>transaction_length</td>
    <td>1 to 9 byte integer // See net_length_size(ulonglong num)</td>
    <td>The packed transaction's length in bytes, including the Gtid</td>
  </tr>
  <tr>
    <td>immediate_server_version</td>
    <td>4 byte integer</td>
    <td>Server version of the immediate server</td>
  </tr>
  <tr>
    <td>original_server_version</td>
    <td>4 byte integer</td>
    <td>Version of the server where the transaction was originally executed</td>
  </tr>
  </table>

*/
class Gtid_event : public Binary_log_event {
 public:
  /*
    The transaction's logical timestamps used for MTS: see
    Transaction_ctx::last_committed and
    Transaction_ctx::sequence_number for details.
    Note: Transaction_ctx is in the MySQL server code.
  */
  long long int last_committed;
  long long int sequence_number;
  /** GTID flags constants */
  unsigned const char FLAG_MAY_HAVE_SBR = 1;
  /** Transaction might have changes logged with SBR */
  bool may_have_sbr_stmts;
  /** Timestamp when the transaction was committed on the originating master. */
  unsigned long long int original_commit_timestamp;
  /** Timestamp when the transaction was committed on the nearest master. */
  unsigned long long int immediate_commit_timestamp;
  bool has_commit_timestamps;
  /** The length of the transaction in bytes. */
  unsigned long long int transaction_length;

 public:
  /**
    Ctor of Gtid_event

    The layout of the buffer is as follows
    <pre>
    +----------+---+---+-------+--------------+---------+----------+
    |gtid flags|SID|GNO|TS_TYPE|logical ts(:s)|commit ts|trx length|
    +----------+---+---+-------+------------------------+----------+
    </pre>
    TS_TYPE is from {G_COMMIT_TS2} singleton set of values
    Details on commit timestamps in Gtid_event(const char*...)

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */

  Gtid_event(const char *buf, const Format_description_event *fde);
  /**
    Constructor.
  */
  explicit Gtid_event(long long int last_committed_arg,
                      long long int sequence_number_arg,
                      bool may_have_sbr_stmts_arg,
                      unsigned long long int original_commit_timestamp_arg,
                      unsigned long long int immediate_commit_timestamp_arg,
                      uint32_t original_server_version_arg,
                      uint32_t immediate_server_version_arg)
      : Binary_log_event(GTID_LOG_EVENT),
        last_committed(last_committed_arg),
        sequence_number(sequence_number_arg),
        may_have_sbr_stmts(may_have_sbr_stmts_arg),
        original_commit_timestamp(original_commit_timestamp_arg),
        immediate_commit_timestamp(immediate_commit_timestamp_arg),
        transaction_length(0),
        original_server_version(original_server_version_arg),
        immediate_server_version(immediate_server_version_arg) {}
#ifndef HAVE_MYSYS
  // TODO(WL#7684): Implement the method print_event_info and print_long_info
  //               for all the events supported  in  MySQL Binlog
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif
 protected:
  static const int ENCODED_FLAG_LENGTH = 1;
  static const int ENCODED_SID_LENGTH = 16;  // Uuid::BYTE_LENGTH;
  static const int ENCODED_GNO_LENGTH = 8;
  /// Length of typecode for logical timestamps.
  static const int LOGICAL_TIMESTAMP_TYPECODE_LENGTH = 1;
  /// Length of two logical timestamps.
  static const int LOGICAL_TIMESTAMP_LENGTH = 16;
  // Type code used before the logical timestamps.
  static const int LOGICAL_TIMESTAMP_TYPECODE = 2;

  static const int IMMEDIATE_COMMIT_TIMESTAMP_LENGTH = 7;
  static const int ORIGINAL_COMMIT_TIMESTAMP_LENGTH = 7;
  // Length of two timestamps (from original/immediate masters)
  static const int FULL_COMMIT_TIMESTAMP_LENGTH =
      IMMEDIATE_COMMIT_TIMESTAMP_LENGTH + ORIGINAL_COMMIT_TIMESTAMP_LENGTH;
  // We use 7 bytes out of which 1 bit is used as a flag.
  static const int ENCODED_COMMIT_TIMESTAMP_LENGTH = 55;
  // Minimum and maximum lengths of transaction length field.
  static const int TRANSACTION_LENGTH_MIN_LENGTH = 1;
  static const int TRANSACTION_LENGTH_MAX_LENGTH = 9;
  /// Length of original_server_version
  static const int ORIGINAL_SERVER_VERSION_LENGTH = 4;
  /// Length of immediate_server_version
  static const int IMMEDIATE_SERVER_VERSION_LENGTH = 4;
  /// Length of original and immediate server version
  static const int FULL_SERVER_VERSION_LENGTH =
      ORIGINAL_SERVER_VERSION_LENGTH + IMMEDIATE_SERVER_VERSION_LENGTH;
  // We use 4 bytes out of which 1 bit is used as a flag.
  static const int ENCODED_SERVER_VERSION_LENGTH = 31;

  /* We have only original commit timestamp if both timestamps are equal. */
  int get_commit_timestamp_length() const {
    if (original_commit_timestamp != immediate_commit_timestamp)
      return FULL_COMMIT_TIMESTAMP_LENGTH;
    return ORIGINAL_COMMIT_TIMESTAMP_LENGTH;
  }

  /**
    We only store the immediate_server_version if both server versions are the
    same.
  */
  int get_server_version_length() const {
    if (original_server_version != immediate_server_version)
      return FULL_SERVER_VERSION_LENGTH;
    return IMMEDIATE_SERVER_VERSION_LENGTH;
  }

  gtid_info gtid_info_struct;
  Uuid Uuid_parent_struct;

  /* Minimum GNO expected in a serialized GTID event */
  static const int64_t MIN_GNO = 1;
  /* Maximum GNO expected in a serialized GTID event */
  static const int64_t MAX_GNO = LLONG_MAX;

 public:
  /// Total length of post header
  static const int POST_HEADER_LENGTH =
      ENCODED_FLAG_LENGTH +               /* flags */
      ENCODED_SID_LENGTH +                /* SID length */
      ENCODED_GNO_LENGTH +                /* GNO length */
      LOGICAL_TIMESTAMP_TYPECODE_LENGTH + /* length of typecode */
      LOGICAL_TIMESTAMP_LENGTH;           /* length of two logical timestamps */

  /*
    We keep the commit timestamps in the body section because they can be of
    variable length.
    On the originating master, the event has only one timestamp as the two
    timestamps are equal. On every other server we have two timestamps.
  */
  static const int MAX_DATA_LENGTH = FULL_COMMIT_TIMESTAMP_LENGTH +
                                     TRANSACTION_LENGTH_MAX_LENGTH +
                                     FULL_SERVER_VERSION_LENGTH;

  static const int MAX_EVENT_LENGTH =
      LOG_EVENT_HEADER_LEN + POST_HEADER_LENGTH + MAX_DATA_LENGTH;
  /**
   Set the transaction length information.

    This function should be used when the full transaction length (including
    the Gtid event length) is known.

    @param transaction_length_arg The transaction length.
  */
  void set_trx_length(unsigned long long int transaction_length_arg) {
    transaction_length = transaction_length_arg;
  }

  /** The version of the server where the transaction was originally executed */
  uint32_t original_server_version;
  /** The version of the immediate server */
  uint32_t immediate_server_version;
};

/**
  @class Previous_gtids_event

  @section Previous_gtids_event_binary_format Binary Format

  The Post-Header for this event type is empty.  The Body has two
  components:

  <table>
  <caption>Body for Previous_gtids_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>buf</td>
    <td>unsigned char array</td>
    <td>It contains the Gtids executed in the
        last binary log file.</td>
  </tr>

  <tr>
    <td>buf_size</td>
    <td>4 byte integer</td>
    <td>Size of the above buffer</td>
  </tr>
  </table>
*/
class Previous_gtids_event : public Binary_log_event {
 public:
  /**
    Decodes the gtid_executed in the last binlog file

    <pre>
    The buffer layout is as follows
    +--------------------------------------------+
    | Gtids executed in the last binary log file |
    +--------------------------------------------+
    </pre>
    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Previous_gtids_event(const char *buf, const Format_description_event *fde);
  /**
    This is the minimal constructor, and set the
    type_code as PREVIOUS_GTIDS_LOG_EVENT in the header object in
    Binary_log_event
  */
  Previous_gtids_event() : Binary_log_event(PREVIOUS_GTIDS_LOG_EVENT) {}
#ifndef HAVE_MYSYS
  // TODO(WL#7684): Implement the method print_event_info and print_long_info
  //               for all the events supported  in  MySQL Binlog
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif
 protected:
  size_t buf_size;
  const unsigned char *buf;
};

/**
  @class Transaction_context_event

  This class is used to combine the information of the ongoing transaction
  including the write set and other information of the thread executing the
  transaction.

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>thread_id</td>
    <td>4 byte integer</td>
    <td>The identifier for the thread executing the transaction.</td>
  </tr>

  <tr>
    <td>gtid_specified</td>
    <td>bool type variable</td>
    <td>Variable to identify whether the Gtid have been specified for the
  ongoing transaction or not.
    </td>
  </tr>

  <tr>
    <td>encoded_snapshot_version</td>
    <td>unsigned char array</td>
    <td>A gtid_set which is used to store the transaction set used for
        conflict detection.</td>
  </tr>

  <tr>
    <td>encoded_snapshot_version_length</td>
    <td>4 byte integer</td>
    <td>Length of the above char array.</td>
  </tr>

  <tr>
    <td>write_set</td>
    <td>variable length list to store the hash values. </td>
    <td>Used to store the hash values of the rows identifier for the rows
        which have changed in the ongoing transaction.
    </td>
  </tr>

  <tr>
    <td>read_set</td>
    <td>variable length list to store the read set values. Currently empty.
  </td> <td>Will be used to store the read set values of the current
  transaction.</td>
  </tr>

*/
class Transaction_context_event : public Binary_log_event {
 public:
  /**
    Decodes the transaction_context_log_event of the ongoing transaction.

    <pre>
    The buffer layout is as follows
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Transaction_context_event(const char *buf,
                            const Format_description_event *fde);

  Transaction_context_event(unsigned int thread_id_arg,
                            bool is_gtid_specified_arg)
      : Binary_log_event(TRANSACTION_CONTEXT_EVENT),
        thread_id(thread_id_arg),
        gtid_specified(is_gtid_specified_arg) {}

  virtual ~Transaction_context_event();

  static const char *read_data_set(const char *pos, uint32_t set_len,
                                   std::list<const char *> *set,
                                   uint32_t remaining_buffer);

  static void clear_set(std::list<const char *> *set);

#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif

 protected:
  const char *server_uuid;
  uint32_t thread_id;
  bool gtid_specified;
  const unsigned char *encoded_snapshot_version;
  uint32_t encoded_snapshot_version_length;
  std::list<const char *> write_set;
  std::list<const char *> read_set;

  // The values mentioned on the next class constants is the offset where the
  // data that will be copied in the buffer.

  // 1 byte length.
  static const int ENCODED_SERVER_UUID_LEN_OFFSET = 0;
  // 4 bytes length.
  static const int ENCODED_THREAD_ID_OFFSET = 1;
  // 1 byte length.
  static const int ENCODED_GTID_SPECIFIED_OFFSET = 5;
  // 4 bytes length
  static const int ENCODED_SNAPSHOT_VERSION_LEN_OFFSET = 6;
  // 4 bytes length.
  static const int ENCODED_WRITE_SET_ITEMS_OFFSET = 10;
  // 4 bytes length.
  static const int ENCODED_READ_SET_ITEMS_OFFSET = 14;

  // The values mentioned on the next class's constants is the length of the
  // data that will be copied in the buffer.
  static const int ENCODED_READ_WRITE_SET_ITEM_LEN = 2;
  static const int ENCODED_SNAPSHOT_VERSION_LEN = 2;
};

/**
  @class View_change_event

  This class is used to add view change markers in the binary log when a
  member of the group enters or leaves the group.

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>view_id</td>
    <td>40 length character array</td>
    <td>This is used to store the view id value of the new view change when a
  node add or leaves the group.
    </td>
  </tr>

  <tr>
    <td>seq_number</td>
    <td>8 bytes integer</td>
    <td>Variable to identify the next sequence number to be alloted to the
  certified transaction.</td>
  </tr>

  <tr>
    <td>certification_info</td>
    <td>variable length map to store the certification data.</td>
    <td>Map to store the certification info ie. the hash of write_set and the
        snapshot sequence value.
    </td>
  </tr>

*/
class View_change_event : public Binary_log_event {
 public:
  /**
    Decodes the view_change_log_event generated incase a server enters or
    leaves the group.

    <pre>
    The buffer layout is as follows
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  View_change_event(const char *buf, const Format_description_event *fde);

  explicit View_change_event(const char *raw_view_id);

  virtual ~View_change_event();

#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif

 protected:
  // The values mentioned on the next class constants is the offset where the
  // data that will be copied in the buffer.

  // 40 bytes length.
  static const int ENCODED_VIEW_ID_OFFSET = 0;
  // 8 bytes length.
  static const int ENCODED_SEQ_NUMBER_OFFSET = 40;
  // 4 bytes length.
  static const int ENCODED_CERT_INFO_SIZE_OFFSET = 48;

  /*
    The layout of the buffer is as follows
    +--------------------- -+-------------+----------+
    | View Id               | seq number  | map size |
    +-----------------------+-------------+----------+
   view id (40 bytes) + seq number (8 bytes) + map size (4 bytes)
   Sum of the length of the values at the above OFFSETS.
  */

  // The values mentioned on the next class constants is the length of the data
  // that will be copied in the buffer.

  // Field sizes on serialization
  static const int ENCODED_VIEW_ID_MAX_LEN = 40;
  static const int ENCODED_CERT_INFO_KEY_SIZE_LEN = 2;
  static const int ENCODED_CERT_INFO_VALUE_LEN = 4;

  char view_id[ENCODED_VIEW_ID_MAX_LEN];

  long long int seq_number;

  std::map<std::string, std::string> certification_info;
};

/**
  @class Heartbeat_event

  Replication event to ensure to slave that master is alive.
  The event is originated by master's dump thread and sent straight to
  slave without being logged. Slave itself does not store it in relay log
  but rather uses a data for immediate checks and throws away the event.

  Two members of the class log_ident and Binary_log_event::log_pos comprise
  @see the rpl_event_coordinates instance. The coordinates that a heartbeat
  instance carries correspond to the last event master has sent from
  its binlog.

  @section Heartbeat_event_binary_format Binary Format

  The Body has one component:

  <table>
  <caption>Body for Heartbeat_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>log_ident</td>
    <td>variable length string without trailing zero, extending to the
    end of the event</td>
    <td>Name of the current binlog being written to.</td>
  </tr>
  </table>
*/
class Heartbeat_event : public Binary_log_event {
 public:
  /**
    Sent by a master to a slave to let the slave know that the master is
    still alive. Events of this type do not appear in the binary or relay logs.
    They are generated on a master server by the thread that dumps events and
    sent straight to the slave without ever being written to the binary log.

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Heartbeat_event(const char *buf, const Format_description_event *fde);

  const char *get_log_ident() { return log_ident; }
  unsigned int get_ident_len() { return ident_len; }

 protected:
  const char *log_ident;
  unsigned int ident_len; /** filename length */
};

}  // end namespace binary_log
/**
  @} (end of group Replication)
*/
#endif /* CONTROL_EVENTS_INCLUDED */
