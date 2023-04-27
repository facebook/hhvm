/*
   Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "shared_memory_connection.h"

#include <errno.h>

#include "channel_info.h"                // Channel_info
#include "connection_handler_manager.h"  // Connection_handler_manager
#include "init_net_server_extension.h"   // init_net_server_extension
#include "my_byteorder.h"
#include "my_shm_defaults.h"
#include "mysql/components/services/log_builtins.h"
#include "sql/log.h"
#include "sql/mysqld.h"  // connection_events_loop_aborted
#include "sql/psi_memory_key.h"
#include "sql/sql_class.h"  // THD
#include "violite.h"        // Vio

///////////////////////////////////////////////////////////////////////////
// Channel_info_shared_mem implementation
///////////////////////////////////////////////////////////////////////////

/**
  This class abstracts the info. about the windows shared memory mode
  of communication with the server.
*/
class Channel_info_shared_mem : public Channel_info {
  HANDLE m_handle_client_file_map;
  char *m_handle_client_map;
  HANDLE m_event_server_wrote;
  HANDLE m_event_server_read;
  HANDLE m_event_client_wrote;
  HANDLE m_event_client_read;
  HANDLE m_event_conn_closed;

 protected:
  virtual Vio *create_and_init_vio() const {
    return vio_new_win32shared_memory(m_handle_client_file_map,
                                      m_handle_client_map, m_event_client_wrote,
                                      m_event_client_read, m_event_server_wrote,
                                      m_event_server_read, m_event_conn_closed);
  }

 public:
  /**
    Constructor to instantiate Channel_info_shared_mem object

    @param   handle_client_file_map  handle to client file map.
    @param   handle_client_map       handle to client map.
    @param   event_server_wrote      handle object for server write event.
    @param   event_server_read       handle object for server read event.
    @param   event_client_wrote      handle object for client write event.
    @param   event_client_read       handle object  for client read event.
    @param   event_conn_closed       handle object for connection close event.
  */
  Channel_info_shared_mem(HANDLE handle_client_file_map,
                          char *handle_client_map, HANDLE event_server_wrote,
                          HANDLE event_server_read, HANDLE event_client_wrote,
                          HANDLE event_client_read, HANDLE event_conn_closed)
      : m_handle_client_file_map(handle_client_file_map),
        m_handle_client_map(handle_client_map),
        m_event_server_wrote(event_server_wrote),
        m_event_server_read(event_server_read),
        m_event_client_wrote(event_client_wrote),
        m_event_client_read(event_client_read),
        m_event_conn_closed(event_conn_closed) {}

  virtual THD *create_thd() {
    THD *thd = Channel_info::create_thd();

    if (thd != NULL) {
      init_net_server_extension(thd);
      thd->security_context()->set_host_ptr(my_localhost, strlen(my_localhost));
    }
    return thd;
  }

  virtual void send_error_and_close_channel(uint errorcode, int error,
                                            bool senderror) {
    Channel_info::send_error_and_close_channel(errorcode, error, senderror);

    // Channel_info::send_error_and_close_channel will have closed
    // handles on senderror
    if (!senderror) {
      if (m_handle_client_file_map) CloseHandle(m_handle_client_file_map);
      if (m_handle_client_map) UnmapViewOfFile(m_handle_client_map);
      if (m_event_server_wrote) CloseHandle(m_event_server_wrote);
      if (m_event_server_read) CloseHandle(m_event_server_read);
      if (m_event_client_wrote) CloseHandle(m_event_client_wrote);
      if (m_event_client_read) CloseHandle(m_event_client_read);
      if (m_event_conn_closed) CloseHandle(m_event_conn_closed);
    }
  }
};

///////////////////////////////////////////////////////////////////////////
// Shared_mem_listener implementation
///////////////////////////////////////////////////////////////////////////

void Shared_mem_listener::close_shared_mem() {
  if (m_temp_buffer) my_free(m_temp_buffer);

  my_security_attr_free(m_sa_event);
  my_security_attr_free(m_sa_mapping);
  if (m_connect_map) UnmapViewOfFile(m_connect_map);
  if (m_connect_named_mutex) CloseHandle(m_connect_named_mutex);
  if (m_connect_file_map) CloseHandle(m_connect_file_map);
  if (m_event_connect_answer) CloseHandle(m_event_connect_answer);
  if (m_event_connect_request) CloseHandle(m_event_connect_request);
}

bool Shared_mem_listener::setup_listener() {
  const char *errmsg = NULL;
  LogErr(INFORMATION_LEVEL, ER_CONN_SHM_LISTENER);
  /*
    get enough space base-name + '_' + longest suffix we might ever send
  */
  if (!(m_temp_buffer =
            (char *)my_malloc(key_memory_shared_memory_name,
                              m_shared_mem_name.length() + 32L, MYF(MY_FAE))))
    goto error;

  if (my_security_attr_create(&m_sa_event, &errmsg, GENERIC_ALL,
                              SYNCHRONIZE | EVENT_MODIFY_STATE))
    goto error;

  if (my_security_attr_create(&m_sa_mapping, &errmsg, GENERIC_ALL,
                              FILE_MAP_READ | FILE_MAP_WRITE))
    goto error;

  /*
    The name of event and file-mapping events create agree next rule:
      shared_memory_base_name+unique_part
    Where:
      shared_memory_base_name is unique value for each server
      unique_part is unique value for each object (events and file-mapping)
  */
  m_suffix_pos = strxmov(m_temp_buffer, m_shared_mem_name.c_str(), "_", NullS);
  my_stpcpy(m_suffix_pos, "CONNECT_REQUEST");
  if ((m_event_connect_request =
           CreateEvent(m_sa_event, false, false, m_temp_buffer)) == 0) {
    errmsg = "Could not create request event";
    goto error;
  }
  my_stpcpy(m_suffix_pos, "CONNECT_ANSWER");
  if ((m_event_connect_answer =
           CreateEvent(m_sa_event, false, false, m_temp_buffer)) == 0) {
    errmsg = "Could not create answer event";
    goto error;
  }

  my_stpcpy(m_suffix_pos, "CONNECT_NAMED_MUTEX");
  m_connect_named_mutex = CreateMutex(NULL, false, m_temp_buffer);
  if (m_connect_named_mutex == NULL) {
    errmsg = "Unable to create connect named mutex.";
    goto error;
  }
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    errmsg = "Another instance of application already running.";
    goto error;
  }

  my_stpcpy(m_suffix_pos, "CONNECT_DATA");
  if ((m_connect_file_map = CreateFileMapping(
           INVALID_HANDLE_VALUE, m_sa_mapping, PAGE_READWRITE, 0,
           sizeof(m_connect_number), m_temp_buffer)) == 0) {
    errmsg = "Could not create file mapping";
    goto error;
  }
  if ((m_connect_map = (char *)MapViewOfFile(m_connect_file_map, FILE_MAP_WRITE,
                                             0, 0, sizeof(DWORD))) == 0) {
    errmsg = "Could not create shared memory service";
    goto error;
  }

  return false;

error:
  if (errmsg) {
    LogErr(ERROR_LEVEL, ER_CONN_SHM_CANT_CREATE_SERVICE, errmsg,
           strerror(errno));
  }

  close_shared_mem();
  return true;
}

Channel_info *Shared_mem_listener::listen_for_connection_event() {
  /* Wait for a request from client */
  WaitForSingleObject(m_event_connect_request, INFINITE);

  /*
    it can be after shutdown command
  */
  if (connection_events_loop_aborted()) return NULL;

  char connect_number_char[22];
  char *p = longlong10_to_str(m_connect_number, connect_number_char, -10);

  /*
    The name of event and file-mapping events create agree next rule:
    shared_memory_base_name+unique_part+number_of_connection
    Where:
      shared_memory_base_name is uniquel value for each server
      unique_part is unique value for each object (events and file-mapping)
      number_of_connection is connection-number between server and client
  */
  m_suffix_pos = strxmov(m_temp_buffer, m_shared_mem_name.c_str(), "_",
                         connect_number_char, "_", NullS);

  const char *errmsg = NULL;
  ulong smem_buffer_length = shared_memory_buffer_length + 4;

  my_stpcpy(m_suffix_pos, "DATA");
  if ((m_handle_client_file_map =
           CreateFileMapping(INVALID_HANDLE_VALUE, m_sa_mapping, PAGE_READWRITE,
                             0, smem_buffer_length, m_temp_buffer)) == 0) {
    errmsg = "Could not create file mapping";
    goto errorconn;
  }
  if ((m_handle_client_map =
           (char *)MapViewOfFile(m_handle_client_file_map, FILE_MAP_WRITE, 0, 0,
                                 smem_buffer_length)) == 0) {
    errmsg = "Could not create memory map";
    goto errorconn;
  }
  my_stpcpy(m_suffix_pos, "CLIENT_WROTE");
  if ((m_event_client_wrote =
           CreateEvent(m_sa_event, false, false, m_temp_buffer)) == 0) {
    errmsg = "Could not create client write event";
    goto errorconn;
  }
  my_stpcpy(m_suffix_pos, "CLIENT_READ");
  if ((m_event_client_read =
           CreateEvent(m_sa_event, false, false, m_temp_buffer)) == 0) {
    errmsg = "Could not create client read event";
    goto errorconn;
  }
  my_stpcpy(m_suffix_pos, "SERVER_READ");
  if ((m_event_server_read =
           CreateEvent(m_sa_event, false, false, m_temp_buffer)) == 0) {
    errmsg = "Could not create server read event";
    goto errorconn;
  }
  my_stpcpy(m_suffix_pos, "SERVER_WROTE");
  if ((m_event_server_wrote =
           CreateEvent(m_sa_event, false, false, m_temp_buffer)) == 0) {
    errmsg = "Could not create server write event";
    goto errorconn;
  }
  my_stpcpy(m_suffix_pos, "CONNECTION_CLOSED");
  if ((m_event_conn_closed =
           CreateEvent(m_sa_event, true, false, m_temp_buffer)) == 0) {
    errmsg = "Could not create closed connection event";
    goto errorconn;
  }
  if (connection_events_loop_aborted()) goto errorconn;

  {
    Channel_info *channel_info = new (std::nothrow) Channel_info_shared_mem(
        m_handle_client_file_map, m_handle_client_map, m_event_server_wrote,
        m_event_server_read, m_event_client_wrote, m_event_client_read,
        m_event_conn_closed);
    if (channel_info != NULL) {
      int4store(m_connect_map, m_connect_number);
      if (!SetEvent(m_event_connect_answer)) {
        errmsg = "Could not send answer event";
        delete channel_info;
        goto errorconn;
      }

      if (!SetEvent(m_event_client_read)) {
        errmsg = "Could not set client to read mode";
        delete channel_info;
        goto errorconn;
      }
      m_connect_number++;
      return channel_info;
    }
  }
errorconn:
  /* Could not form connection;  Free used handlers/memort and retry */
  if (errmsg) {
    LogErr(ERROR_LEVEL, ER_CONN_SHM_CANT_CREATE_CONNECTION, errmsg,
           strerror(errno));
  }
  if (m_handle_client_file_map) CloseHandle(m_handle_client_file_map);
  if (m_handle_client_map) UnmapViewOfFile(m_handle_client_map);
  if (m_event_server_wrote) CloseHandle(m_event_server_wrote);
  if (m_event_server_read) CloseHandle(m_event_server_read);
  if (m_event_client_wrote) CloseHandle(m_event_client_wrote);
  if (m_event_client_read) CloseHandle(m_event_client_read);
  if (m_event_conn_closed) CloseHandle(m_event_conn_closed);
  return NULL;
}

void Shared_mem_listener::close_listener() {
  if (!SetEvent(m_event_connect_request)) {
    DBUG_PRINT("error",
               ("Got error: %ld from SetEvent of mem_event_connect_request",
                GetLastError()));
  }
  close_shared_mem();
}
