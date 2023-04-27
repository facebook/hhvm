/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <mysql.h>  // for MYSQL structure

#include "handshake.h"
#include "my_dbug.h"

/// Client-side context for authentication handshake

class Handshake_client : public Handshake {
  /**
    Name of the server's service for which we authenticate.

    The service name is sent by server in the initial packet. If no
    service name is used, this member is @c NULL.
  */
  SEC_WCHAR *m_service_name;

  /// Buffer for storing service name obtained from server.
  SEC_WCHAR m_service_name_buf[MAX_SERVICE_NAME_LENGTH];

  Connection &m_con;

 public:
  Handshake_client(Connection &con, const char *target, size_t len);
  ~Handshake_client();

  Blob first_packet();
  Blob process_data(const Blob &);

  Blob read_packet();
  int write_packet(Blob &data);
};

/**
  Create authentication handshake context for client.

  @param con     connection for communication with the peer
  @param target  name of the target service with which we will authenticate
                 (can be NULL if not used)
  @param len     length of target

  Some security packages (like Kerberos) require providing explicit name
  of the service with which a client wants to authenticate. The server-side
  authentication plugin sends this name in the greeting packet
  (see @c win_auth_handshake_{server,client}() functions).
*/

Handshake_client::Handshake_client(Connection &con, const char *target,
                                   size_t len)
    : Handshake(SSP_NAME, CLIENT), m_service_name(NULL), m_con(con) {
  if (!target || 0 == len) return;

  // Convert received UPN to internal WCHAR representation.

  m_service_name = utf8_to_wchar(target, &len);

  if (m_service_name)
    DBUG_PRINT("info", ("Using target service: %S\n", m_service_name));
  else {
    /*
      Note: we ignore errors here - m_target will be NULL, the target name
      will not be used and system will fall-back to NTLM authentication. But
      we leave trace in error log.
    */
    ERROR_LOG(WARNING, ("Could not decode UPN sent by the server"
                        "; target service name will not be used"
                        " and Kerberos authentication will not work"));
  }
}

Handshake_client::~Handshake_client() {
  if (m_service_name) free(m_service_name);
}

Blob Handshake_client::read_packet() {
  /*
    We do a fake read in the first round because first
    packet from the server containing UPN must be read
    before the handshake context is created and the packet
    processing loop starts. We return an empty blob here
    and process_data() function will ignore it.
  */
  if (m_round == 1) return Blob();

  // Otherwise we read packet from the connection.

  Blob packet = m_con.read();
  m_error = m_con.error();
  if (!m_error && packet.is_null())
    m_error = true;  // (no specific error code assigned)

  if (m_error) return Blob();

  DBUG_PRINT("dump", ("Got the following bytes"));
  DBUG_DUMP("dump", packet.ptr(), packet.len());
  return packet;
}

int Handshake_client::write_packet(Blob &data) {
  /*
   Length of the first data payload send by client authentication plugin is
   limited to 255 bytes (because it is wrapped inside client authentication
   packet and is length-encoded with 1 byte for the length).

   If the data payload is longer than 254 bytes, then it is sent in two parts:
   first part of length 255 will be embedded in the authentication packet,
   second part will be sent in the following packet. Byte 255 of the first
   part contains information about the total length of the payload. It is a
   number of blocks of size 512 bytes which is sufficient to store the
   combined packets.

   Server's logic for reading first client's payload is as follows
   (see Handshake_server::read_packet()):
   1. Read data from the authentication packet, if it is shorter than 255 bytes
      then that is all data sent by client.
   2. If there is 255 bytes of data in the authentication packet, read another
      packet and append it to the data, skipping byte 255 of the first packet
      which can be used to allocate buffer of appropriate size.
  */

  size_t len2 = 0;  // length of the second part of first data payload
  byte saved_byte;  // for saving byte 255 in which data length is stored

  if (m_round == 1 && data.len() > 254) {
    len2 = data.len() - 254;
    DBUG_PRINT("info", ("Splitting first packet of length %lu"
                        ", %lu bytes will be sent in a second part",
                        data.len(), len2));
    /*
      Store in byte 255 the number of 512b blocks that are needed to
      keep all the data.
    */
    unsigned block_count = data.len() / 512 + ((data.len() % 512) ? 1 : 0);

#if !defined(DBUG_OFF) && defined(WINAUTH_USE_DBUG_LIB)

    /*
      For testing purposes, use wrong block count to see how server
      handles this.
    */
    DBUG_EXECUTE_IF("winauth_first_packet_test", {
      block_count = data.len() == 601 ? 0 : data.len() == 602 ? 1 : block_count;
    });

#endif

    DBUG_ASSERT(block_count < (unsigned)0x100);
    saved_byte = data[254];
    data[254] = block_count;

    data.trim(255);
  }

  DBUG_PRINT("dump", ("Sending the following data"));
  DBUG_DUMP("dump", data.ptr(), data.len());
  int ret = m_con.write(data);

  if (ret) return ret;

  // Write second part if it is present.
  if (len2) {
    data[254] = saved_byte;
    Blob data2(data.ptr() + 254, len2);
    DBUG_PRINT("info", ("Sending second part of data"));
    DBUG_DUMP("info", data2.ptr(), data2.len());
    ret = m_con.write(data2);
  }

  return ret;
}

/**
  Process data sent by server.

  @param[in]  data  blob with data from server

  This method analyses data sent by server during authentication handshake.
  If client should continue packet exchange, this method returns data to
  be sent to the server next. If no more data needs to be exchanged, an
  empty blob is returned and @c is_complete() is @c true. In case of error
  an empty blob is returned and @c error() gives non-zero error code.

  When invoked for the first time (in the first round of the handshake)
  there is no data from the server (data blob is null) and the intial
  packet is generated without an input.

  @return Data to be sent to the server next or null blob if no more data
  needs to be exchanged or in case of error.
*/

Blob Handshake_client::process_data(const Blob &data) {
#if !defined(DBUG_OFF) && defined(WINAUTH_USE_DBUG_LIB)
  /*
    Code for testing the logic for sending the first client payload.

    A fake data of length given by environment variable TEST_PACKET_LENGTH
    (or default 255 bytes) is sent to the server. First 2 bytes of the
    payload contain its total length (LSB first). The length of test data
    is limited to 2048 bytes.

    Upon receiving test data, server will check that data is correct and
    refuse connection. If server detects data errors it will crash on
    assertion.

    This code is executed if debug flag "winauth_first_packet_test" is
    set, e.g. using client option:

     --debug="d,winauth_first_packet_test"

     The same debug flag must be enabled in the server, e.g. using
     statement:

     SET GLOBAL debug= '+d,winauth_first_packet_test';
  */

  static byte test_buf[2048];

  if (m_round == 1 &&
      DBUG_EVALUATE_IF("winauth_first_packet_test", true, false)) {
    const char *env = getenv("TEST_PACKET_LENGTH");
    size_t len = env ? atoi(env) : 0;
    if (!len) len = 255;
    if (len > sizeof(test_buf)) len = sizeof(test_buf);

    // Store data length in first 2 bytes.
    byte *ptr = test_buf;
    *ptr++ = len & 0xFF;
    *ptr++ = len >> 8;

    // Fill remaining bytes with known values.
    for (byte b = 0; ptr < test_buf + len; ++ptr, ++b) *ptr = b;

    return Blob(test_buf, len);
  };

#endif

  Security_buffer input(data);
  SECURITY_STATUS ret;

  m_output.mem_free();

  ret = InitializeSecurityContextW(
      &m_cred,
      m_round == 1 ? NULL : &m_sctx,  // partial context
      m_service_name,                 // service name
      ASC_REQ_ALLOCATE_MEMORY,        // requested attributes
      0,                              // reserved
      SECURITY_NETWORK_DREP,          // data representation
      m_round == 1 ? NULL : &input,   // input data
      0,                              // reserved
      &m_sctx,                        // context
      &m_output,                      // output data
      &m_atts,                        // attributes
      &m_expire);                     // expire date

  if (process_result(ret)) {
    DBUG_PRINT("error",
               ("InitializeSecurityContext() failed with error %X", ret));
    return Blob();
  }

  return m_output.as_blob();
}

/**********************************************************************/

/* clang-format off */
/**
  @page page_protocol_connection_phase_authentication_methods_authentication_windows Windows Native Authentication

  Authentication::WindowsAuth:

  <ul>
  <li>
  The server name is *authentication_windows*
  </li>
  <li>
  The client name is *authentication_windows_client*
  </li>
  </ul>

  The Windows Native Authentication method is more complex than the other
  methods and extends the auth protocol as it has to send more data forth
  and back than the old handshake permitted.

  Basically it wraps the output of the
  [Negotiate SSP]("http://msdn.microsoft.com/en-us/library/windows/desktop/aa378748(v=VS.85).aspx")
  in the Auth Phase protocol which either means
  @ref sect_protocol_connection_phase_authentication_methods_authentication_windows_ntlm or
  @ref sect_protocol_connection_phase_authentication_methods_authentication_windows_spnego
  are used as underlying protocol.

  Due to the implementation details the Windows Native Authentication method
  doesn't use the fast path of the @ref page_protocol_connection_phase, but is
  only triggered on request as part of the
  @ref page_protocol_connection_phase_packets_protocol_auth_switch_request packet.


  @note Due to implementation details (again) the first packet sent from the
  client to the server is expected to be either
  <ul><li>254 bytes long max or</li>
  <li>send the first 254 bytes first, appended by 1 byte with a magic value
  plus a 2nd packet with rest of the data</li></ul>
  Also following windows authentication packets don't get split.

  The client will send either a
  @ref sect_protocol_connection_phase_authentication_methods_authentication_windows_spnego
  or a @ref sect_protocol_connection_phase_authentication_methods_authentication_windows_ntlm
  packet as a next packet.

  To implement the protocol one can use several existing implementations:
  <ul>
  <li>MS Windows provides
  [InitializeSecurityContextW](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375509(v=VS.85).aspx)
  and [AcceptSecurityContext](http://msdn.microsoft.com/en-us/library/aa374703.aspx)
  </li>
  <li>A open source implemenation of NTML, SPNEGO and Kerberos5 are provided by
  [Heimdal](http://www.h5l.org/)
  </li>
  <li>Java6 added SPNEGO support to
  [JGSS](http://download.oracle.com/javase/6/docs/technotes/guides/security/jgss/lab/part5.html#SPNEGO)
  which also provides the NTLM and Kerberos5 support.
  </li></ul>

  @sa win_auth_handshake_client


  @section sect_protocol_connection_phase_authentication_methods_authentication_windows_ntlm NTLM

  @note [Removed in Windows Vista and 2008](http://msdn.microsoft.com/en-us/library/aa480152.aspx#appcomp_topic16)

  @note Documented in [MSDN](https://msdn.microsoft.com/en-us/library/cc207842.aspx)

  @startuml
  Client->Server: NTLM request
  Server->Client: 0x01 + NTLM response
  == repeat until done ==
  Client->Server: NTLM request
  Server->Client: OK
  @enduml


  @section sect_protocol_connection_phase_authentication_methods_authentication_windows_spnego SPNEGO

  Uses GSS-API as protocol and negotiates the proper auth-method automatically.
  @par Tip
  To decode these packets by hand you need to read:
    <ul>
      <li>
         [RFC2473](http://tools.ietf.org/html/rfc2743.html#page-81)
         Section 3.1: Mechanism-independent Token Format
      </li><li>
         [RFC4178](http://tools.ietf.org/html/rfc4178.html#page-7)
         Section 4: Token Defintions
      </li><li>
         [X.680](http://www.itu.int/ITU-T/studygroups/com17/languages/X.680-0207.pdf)
         ASN.1
      </li><li>
         [X.690](http://www.itu.int/ITU-T/studygroups/com17/languages/X.690-0207.pdf)
         DER
      </li>
    </ul>

  @startuml
  Client->Server: GSS-API + SPNEGO NegTokenInit
  Server->Client: 0x01 + SPNEGO NegTokenResponse
  Client->Server: SPNEGO NegTokenResponse
  Server->Client: 0x01 + SPNEGO NegTokenResponse
  == repeat until done ==
  Server->Client: OK
  @enduml

*/
/* clang-format on */

/**
  Perform authentication handshake from client side.

  @param[in]  vio    pointer to @c MYSQL_PLUGIN_VIO instance to be used
                     for communication with the server
  @param[in]  mysql  pointer to a MySQL connection for which we authenticate

  After reading the initial packet from server, containing its UPN to be
  used as service name, client starts packet exchange by sending the first
  packet in this exchange. While handshake is not yet completed, client
  reads packets sent by the server and process them, possibly generating new
  data to be sent to the server.

  This function reports errors.

  @return 0 on success.
*/

int win_auth_handshake_client(MYSQL_PLUGIN_VIO *vio, MYSQL *mysql) {
  DBUG_TRACE;

  /*
    Check if we should enable logging.
  */
  {
    const char *opt = getenv("AUTHENTICATION_WIN_LOG");
    int opt_val = opt ? atoi(opt) : 0;
    if (opt && !opt_val) {
      if (!_strnicmp("on", opt, 2)) opt_val = 2;
      if (!_strnicmp("yes", opt, 3)) opt_val = 2;
      if (!_strnicmp("true", opt, 4)) opt_val = 2;
      if (!_strnicmp("debug", opt, 5)) opt_val = 4;
      if (!_strnicmp("dbug", opt, 4)) opt_val = 4;
    }
    set_log_level(opt_val);
  }

  ERROR_LOG(INFO, ("Authentication handshake for account %s", mysql->user));

  // Create connection object.

  Connection con(vio);
  DBUG_ASSERT(!con.error());

  // Read initial packet from server containing service name.

  Blob service_name = con.read();

  if (con.error() || service_name.is_null()) {
    ERROR_LOG(ERROR, ("Error reading initial packet"));
    return CR_ERROR;
  }
  DBUG_PRINT("info", ("Got initial packet of length %d", service_name.len()));

  // Create authentication handshake context using the given service name.

  Handshake_client hndshk(con,
                          service_name[0] ? (char *)service_name.ptr() : NULL,
                          service_name.len());
  if (hndshk.error()) {
    ERROR_LOG(ERROR, ("Could not create authentication handshake context"));
    return CR_ERROR;
  }

  DBUG_ASSERT(!hndshk.error());

  /*
    Read and process packets from server until handshake is complete.
    Note that the first read from server is dummy
    (see Handshake_client::read_packet()) as we already have read the
    first packet to establish service name.
  */
  if (hndshk.packet_processing_loop()) return CR_ERROR;

  DBUG_ASSERT(!hndshk.error() && hndshk.is_complete());

  return CR_OK;
}
