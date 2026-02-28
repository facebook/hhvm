#ifndef PROTOCOL_CLASSIC_INCLUDED
#define PROTOCOL_CLASSIC_INCLUDED

/* Copyright (c) 2002, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>
#include <sys/types.h>

#include "field_types.h"  // enum_field_types
#include "m_ctype.h"
#include "my_command.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "mysql_com.h"
#include "mysql_time.h"
#include "sql/protocol.h"  // Protocol
#include "violite.h"

class Item_param;
class Send_field;
class String;
class i_string;
class my_decimal;
template <class T>
class I_List;
template <class T>
class List;
union COM_DATA;
class THD;

class Protocol_classic : public Protocol {
 private:
  ulong m_client_capabilities;
  bool parse_packet(union COM_DATA *data, enum_server_command cmd);
  bool net_store_data_with_conversion(const uchar *from, size_t length,
                                      const CHARSET_INFO *fromcs,
                                      const CHARSET_INFO *tocs);

 protected:
  THD *m_thd;
  String *packet;
  String convert;
  uint field_pos;
  bool send_metadata;
#ifndef DBUG_OFF
  enum enum_field_types *field_types;
  uint count;
#endif
  uint field_count;
  uint sending_flags;
  ulong input_packet_length;
  uchar *input_raw_packet;
  unsigned long checksum;
  bool should_record_checksum;
  const CHARSET_INFO *result_cs;

  bool send_ok(uint server_status, uint statement_warn_count,
               ulonglong affected_rows, ulonglong last_insert_id,
               const char *message) override;

  bool send_eof(uint server_status, uint statement_warn_count) override;

  bool send_error(uint sql_errno, const char *err_msg,
                  const char *sql_state) override;
  bool store_ps_status(ulong stmt_id, uint column_count, uint param_count,
                       ulong cond_count) override;

 public:
  bool bad_packet;
  Protocol_classic()
      : send_metadata(false),
        input_packet_length(0),
        checksum(0),
        should_record_checksum(false),
        bad_packet(true) {}
  Protocol_classic(THD *thd)
      : send_metadata(false),
        input_packet_length(0),
        input_raw_packet(nullptr),
        checksum(0),
        should_record_checksum(false),
        bad_packet(true) {
    init(thd);
  }
  void init(THD *thd_arg);
  bool store_field(const Field *field) final override;
  bool store_string(const char *from, size_t length,
                    const CHARSET_INFO *cs) final;
  int read_packet() override;
  int get_command(COM_DATA *com_data, enum_server_command *cmd) override;
  /**
    Parses the passed parameters and creates a command

    @param com_data  out parameter
    @param cmd       in  parameter
    @param pkt       packet to be parsed
    @param length    size of the packet

    @retval false   ok
    @retval true    error
  */
  bool create_command(COM_DATA *com_data, enum_server_command cmd, uchar *pkt,
                      size_t length);
  bool flush() override;
  void end_partial_result_set() override;

  bool end_row() override;
  uint get_rw_status() override;
  bool get_compression() override;

  char *get_compression_algorithm() override;
  uint get_compression_level() override;

  bool start_result_metadata(uint num_cols, uint flags,
                             const CHARSET_INFO *resultcs) override;
  bool end_result_metadata() override;
  bool send_field_metadata(Send_field *field,
                           const CHARSET_INFO *item_charset) override;
  virtual bool store_result_set_metadata_object_names(Send_field *field);
  void abort_row() override {}
  virtual void gen_conn_timeout_err(char *msg_buf) override;

  /**
    Returns the type of the connection

    @return
      enum enum_vio_type
  */
  enum enum_vio_type connection_type() const override {
    const Vio *v = get_vio();
    return v ? vio_type(v) : NO_VIO_TYPE;
  }

  my_socket get_socket();
  // NET interaction functions
  /* Initialize NET */
  bool init_net(Vio *vio);
  void claim_memory_ownership();
  /* Deinitialize NET */
  void end_net();
  /* Write data to NET buffer */
  bool write(const uchar *ptr, size_t len);
  /* Return last error from NET */
  uchar get_error();
  /* Set max allowed packet size */
  void set_max_packet_size(ulong max_packet_size);
  /* Deinitialize VIO */
  int shutdown(bool server_shutdown = false) override;
  /* Wipe NET with zeros */
  void wipe_net();
  /* Check whether VIO is healhty */
  bool connection_alive() const override;
  /* Returns the client capabilities */
  ulong get_client_capabilities() override { return m_client_capabilities; }
  /* Sets the client capabilities */
  void set_client_capabilities(ulong client_capabilities) {
    this->m_client_capabilities = client_capabilities;
  }
  /* Adds  client capability */
  void add_client_capability(ulong client_capability) {
    m_client_capabilities |= client_capability;
  }
  /* Removes a client capability*/
  void remove_client_capability(unsigned long capability) {
    m_client_capabilities &= ~capability;
  }
  /* Returns true if the client has the capability and false otherwise*/
  bool has_client_capability(unsigned long client_capability) override {
    return (bool)(m_client_capabilities & client_capability);
  }
  /* Set the checksum response attribute */
  void record_checksum();
  // TODO: temporary functions. Will be removed.
  // DON'T USE IN ANY NEW FEATURES.
  /* Return NET */
  NET *get_net();
  /* return VIO */
  Vio *get_vio();
  const Vio *get_vio() const;
  /* Set VIO */
  void set_vio(Vio *vio);
  /* Set packet number */
  void set_output_pkt_nr(uint pkt_nr);
  /* Return packet number */
  uint get_output_pkt_nr();
  /* Return packet string */
  String *get_output_packet();
  /* return packet length */
  uint get_packet_length() { return input_packet_length; }
  /* Return raw packet buffer */
  uchar *get_raw_packet() { return input_raw_packet; }
  /* Set read timeout */
  virtual void set_read_timeout(ulong read_timeout);
  /* Set write timeout */
  virtual void set_write_timeout(ulong write_timeout);

  /**
   * Sets the character set expected by the client. This function is for unit
   * tests. It should usually be set by calling start_result_metadata().
   */
  void set_result_character_set(const CHARSET_INFO *charset) {
    result_cs = charset;
  }
};

/** Class used for the old (MySQL 4.0 protocol). */

class Protocol_text : public Protocol_classic {
 public:
  Protocol_text() {}
  Protocol_text(THD *thd_arg) : Protocol_classic(thd_arg) {}
  bool store_null() override;
  bool store_tiny(longlong from, uint32 zerofill) override;
  bool store_short(longlong from, uint32 zerofill) override;
  bool store_long(longlong from, uint32 zerofill) override;
  bool store_longlong(longlong from, bool unsigned_flag,
                      uint32 zerofill) override;
  bool store_decimal(const my_decimal *, uint, uint) final;
  bool store_float(float nr, uint32 decimals, uint32 zerofill) override;
  bool store_double(double from, uint32 decimals, uint32 zerofill) override;
  bool store_datetime(const MYSQL_TIME &time, uint precision) override;
  bool store_date(const MYSQL_TIME &time) override;
  bool store_time(const MYSQL_TIME &time, uint precision) override;
  void start_row() override;
  bool send_parameters(List<Item_param> *parameters, bool) override;

  enum enum_protocol_type type() const override { return PROTOCOL_TEXT; }
};

class Protocol_binary final : public Protocol_text {
 private:
  uint bit_fields;

 public:
  Protocol_binary() {}
  Protocol_binary(THD *thd_arg) : Protocol_text(thd_arg) {}
  void start_row() override;
  bool store_null() override;
  bool store_tiny(longlong from, uint32 zerofill) override;
  bool store_short(longlong from, uint32 zerofill) override;
  bool store_long(longlong from, uint32 zerofill) override;
  bool store_longlong(longlong from, bool unsigned_flag,
                      uint32 zerofill) override;
  // Decimals are sent as text, also over the binary protocol.
  using Protocol_text::store_decimal;
  bool store_datetime(const MYSQL_TIME &time, uint precision) override;
  bool store_date(const MYSQL_TIME &time) override;
  bool store_time(const MYSQL_TIME &time, uint precision) override;
  bool store_float(float nr, uint32 decimals, uint32 zerofill) override;
  bool store_double(double from, uint32 decimals, uint32 zerofill) override;

  bool start_result_metadata(uint num_cols, uint flags,
                             const CHARSET_INFO *resultcs) override;
  bool send_parameters(List<Item_param> *parameters,
                       bool is_sql_prepare) override;

  enum enum_protocol_type type() const override { return PROTOCOL_BINARY; }
};

bool net_send_error(THD *thd, uint sql_errno, const char *err);
bool net_send_error(NET *net, uint sql_errno, const char *err);
uchar *net_store_data(uchar *to, const uchar *from, size_t length);
bool store(Protocol *prot, I_List<i_string> *str_list);
#endif /* PROTOCOL_CLASSIC_INCLUDED */
