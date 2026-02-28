/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_ASYNC_INCLUDED
#define MYSQL_ASYNC_INCLUDED

#define MYSQL_ASYNC_INCLUDED

#include <mysql.h>

/*
  NOTE:this file should not be included as part of packaging.
*/
/* Async MySQL extension fields here. */

/*
  This enum is to represent different asynchronous operations like reading the
  network, writing to network, idle state, or complete state.
*/
enum net_async_operation {
  NET_ASYNC_OP_IDLE = 0, /* default state */
  NET_ASYNC_OP_READING,  /* used by my_net_read calls */
  NET_ASYNC_OP_WRITING,  /* used by my_net_write calls */
  NET_ASYNC_OP_COMPLETE  /* network read or write is complete */
};

/* Reading a packet is a multi-step process, so we have a state machine. */
enum net_async_read_packet_state {
  NET_ASYNC_PACKET_READ_IDLE = 0, /* default packet read state */
  NET_ASYNC_PACKET_READ_HEADER,   /* read packet header */
  NET_ASYNC_PACKET_READ_BODY,     /* read packet contents */
  NET_ASYNC_PACKET_READ_COMPLETE  /* state to define if packet is
                                     completely read */
};

/* Different states when reading a query result. */
enum net_read_query_result_status {
  NET_ASYNC_READ_QUERY_RESULT_IDLE = 0,    /* default state */
  NET_ASYNC_READ_QUERY_RESULT_FIELD_COUNT, /* read Ok or read field
                                              count sent as part of
                                              COM_QUERY */
  NET_ASYNC_READ_QUERY_RESULT_FIELD_INFO   /* read result of above
                                              COM_* command */
};

/* Sending a command involves the write as well as reading the status. */
enum net_send_command_status {
  NET_ASYNC_SEND_COMMAND_IDLE = 0,      /* default send command state */
  NET_ASYNC_SEND_COMMAND_WRITE_COMMAND, /* send COM_* command */
  NET_ASYNC_SEND_COMMAND_READ_STATUS    /* read result of above COM_*
                                           command */
};

/*
  Async operations are broadly classified into 3 phases:
  Connection phase, phase of sending data to server (which is writing phase)
  and reading data from server (which is reading phase). Below enum describes
  the same
*/
enum net_async_block_state {
  NET_NONBLOCKING_CONNECT = 0,
  NET_NONBLOCKING_READ,
  NET_NONBLOCKING_WRITE
};

struct io_vec {
  void *iov_base; /* Starting address */
  size_t iov_len; /* Number of bytes to transfer */
};

#define ASYNC_COALESCE_BUFFER_SIZE 1024

// Make sure the coalesce buffer size is larger than we used to have
#define ASYNC_INLINE_ORIGINAL_SIZE \
  NET_HEADER_SIZE + COMP_HEADER_SIZE + NET_HEADER_SIZE + 1 + 1
#if ASYNC_COALESCE_BUFFER_SIZE < ASYNC_INLINE_ORIGINAL_SIZE
#error The coalesce buffer size is too small
#endif

typedef struct NET_ASYNC {
  /* The position in buff we continue reads from when data is next available */
  unsigned char *cur_pos;
  /** Blocking state */
  enum net_async_block_state async_blocking_state;
  /** Our current operation */
  enum net_async_operation async_operation;
  /** How many bytes we want to read */
  size_t async_bytes_wanted;
  /*
    Simple state to know if we're reading the first row, and
    command/query statuses.
  */
  bool read_rows_is_first_read;
  enum net_send_command_status async_send_command_status;
  enum net_read_query_result_status async_read_query_result_status;

  /* State when waiting on an async read */
  enum net_async_read_packet_state async_packet_read_state;
  /* Size of the packet we're currently reading */
  size_t async_packet_length;
  size_t async_packet_uncompressed_length;
  size_t multi_packet_offset;

  /*
    Headers and vector for our async writes; see net_serv.c for
    detailed description.
  */
  unsigned char *async_write_headers;
  struct io_vec *async_write_vector;
  size_t async_write_vector_size;
  size_t async_write_vector_current;
  unsigned char inline_async_write_header[ASYNC_COALESCE_BUFFER_SIZE];
  struct io_vec inline_async_write_vector[3];
  unsigned char **compressed_write_buffers;
  size_t compressed_buffers_size;

  /* State for reading responses that are larger than MAX_PACKET_LENGTH */
  unsigned long async_multipacket_read_saved_whereb;
  unsigned long async_multipacket_read_total_len;
  bool async_multipacket_read_started;
} NET_ASYNC;

typedef struct NET_EXTENSION {
  NET_ASYNC *net_async_context;
  mysql_compress_context compress_ctx;
} NET_EXTENSION;

NET_EXTENSION *net_extension_init();
void net_extension_free(NET *);

#define NET_EXTENSION_PTR(N) \
  ((NET_EXTENSION *)((N)->extension ? (N)->extension : NULL))

#define NET_ASYNC_DATA(M) \
  ((NET_EXTENSION_PTR(M)) ? (NET_EXTENSION_PTR(M)->net_async_context) : NULL)

/*
  Asynchronous operations are broadly classified into 2 categories.
  1. Connection
  2. Query execution
  This classification is defined in below enum
*/
enum mysql_async_operation_status {
  ASYNC_OP_UNSET = 0,
  ASYNC_OP_CONNECT,
  ASYNC_OP_QUERY
};

/*
  Query execution in an asynchronous fashion is broadly divided into 3 states
  which is described in below enum
*/
enum mysql_async_query_state_enum {
  QUERY_IDLE = 0,
  QUERY_SENDING,
  QUERY_READING_RESULT
};

typedef struct MYSQL_ASYNC {
  /* Buffer storing the rows result for cli_read_rows_nonblocking */
  MYSQL_DATA *rows_result_buffer;
  /* a pointer to keep track of the previous row of the current result row */
  MYSQL_ROWS **prev_row_ptr;
  /* Context needed to track the state of a connection being established */
  struct mysql_async_connect *connect_context;
  /* Status of the current async op */
  enum mysql_async_operation_status async_op_status;
  /* Size of the current running async query */
  size_t async_query_length;
  /* If a query is running, this is its state */
  enum mysql_async_query_state_enum async_query_state;
  /* context needed to support metadata read operation */
  unsigned long *async_read_metadata_field_len;
  MYSQL_FIELD *async_read_metadata_fields;
  MYSQL_ROWS async_read_metadata_data;
  unsigned int async_read_metadata_cur_field;
  /* a pointer to keep track of the result sets */
  struct MYSQL_RES *async_store_result_result;
} MYSQL_ASYNC;

enum net_async_status my_net_write_nonblocking(NET *net,
                                               const unsigned char *packet,
                                               size_t len, bool *res);
enum net_async_status net_write_command_nonblocking(
    NET *net, unsigned char command, const unsigned char *prefix,
    size_t prefix_len, const unsigned char *packet, size_t packet_len,
    bool *res);
enum net_async_status my_net_read_nonblocking(NET *net, unsigned long *len_ptr,
                                              unsigned long *complen_ptr);

#endif /* MYSQL_ASYNC_INCLUDED */
