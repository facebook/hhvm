/* Copyright (c) 2011, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "handshake.h"

/** Handshake class implementation **********************************/

/**
  Create common part of handshake context.

  @param[in]  ssp   name of the SSP (Security Service Provider) to
                    be used for authentication
  @param[in]  side  is this handshake object used for server- or
                    client-side handshake

  Prepare for handshake using the @c ssp security module. We use
  "Negotiate" which picks best available module. Parameter @c side
  tells if this is preparing for server or client side authentication
  and is used to prepare appropriate credentials.
*/

Handshake::Handshake(const char *ssp, side_t side)
    : m_atts(0L),
      m_error(0),
      m_complete(false),
      m_have_credentials(false),
      m_have_sec_context(false)
#ifndef DBUG_OFF
      ,
      m_ssp_info(NULL)
#endif
{
  SECURITY_STATUS ret;

  // Obtain credentials for the authentication handshake.

  ret = AcquireCredentialsHandle(
      NULL, (SEC_CHAR *)ssp,
      side == SERVER ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND, NULL, NULL,
      NULL, NULL, &m_cred, &m_expire);

  if (ret != SEC_E_OK) {
    DBUG_PRINT("error", ("AcqireCredentialsHandle() failed"
                         " with error %X",
                         ret));
    ERROR_LOG(ERROR, ("Could not obtain local credentials"
                      " required for authentication"));
    m_error = ret;
  }

  m_have_credentials = true;
}

Handshake::~Handshake() {
  if (m_have_credentials) FreeCredentialsHandle(&m_cred);
  if (m_have_sec_context) DeleteSecurityContext(&m_sctx);
  m_output.mem_free();

#ifndef DBUG_OFF
  if (m_ssp_info) FreeContextBuffer(m_ssp_info);
#endif
}

/**
  Read and process data packets from the other end of a connection.

  Packets are read and processed until authentication handshake is
  complete. It is assumed that the peer will send at least one packet.
  Packets are processed with @c process_data() method. If new data is
  generated during packet processing, this data is sent to the peer and
  another round of packet exchange starts.

  @retval 0 on success.

  @note In case of error, appropriate error message is logged.
*/
int Handshake::packet_processing_loop() {
  m_round = 0;

  do {
    ++m_round;
    // Read packet send by the peer

    DBUG_PRINT("info", ("Waiting for packet"));
    Blob packet = read_packet();
    if (error()) {
      ERROR_LOG(ERROR, ("Error reading packet in round %d", m_round));
      return 1;
    }
    DBUG_PRINT("info", ("Got packet of length %d", packet.len()));

    /*
      Process received data, possibly generating new data to be sent.
    */

    Blob new_data = process_data(packet);

    if (error()) {
      ERROR_LOG(ERROR, ("Error processing packet in round %d", m_round));
      return 1;
    }

    /*
      If new data has been generated, send it to the peer. Otherwise
      handshake must be completed.
    */

    if (!new_data.is_null()) {
      DBUG_PRINT("info", ("Round %d started", m_round));

      DBUG_PRINT("info", ("Sending packet of length %d", new_data.len()));
      int ret = write_packet(new_data);
      if (ret) {
        ERROR_LOG(ERROR, ("Error writing packet in round %d", m_round));
        return 1;
      }
      DBUG_PRINT("info", ("Data sent"));
    } else if (!is_complete()) {
      ERROR_LOG(ERROR, ("No data to send in round %d"
                        " but handshake is not complete",
                        m_round));
      return 1;
    }

    /*
      To protect against malicious clients, break handshake exchange if
      too many rounds.
    */

    if (m_round > MAX_HANDSHAKE_ROUNDS) {
      ERROR_LOG(ERROR, ("Authentication handshake could not be completed"
                        " after %d rounds",
                        m_round));
      return 1;
    }

  } while (!is_complete());

  ERROR_LOG(INFO, ("Handshake completed after %d rounds", m_round));
  return 0;
}

#ifndef DBUG_OFF

/**
  Get name of the security package which was used in authentication.

  This method should be called only after handshake was completed. It is
  available only in debug builds.

  @return Name of security package or NULL if it can not be obtained.
*/

const char *Handshake::ssp_name() {
  if (!m_ssp_info && m_complete) {
    SecPkgContext_PackageInfo pinfo;

    int ret = QueryContextAttributes(&m_sctx, SECPKG_ATTR_PACKAGE_INFO, &pinfo);

    if (SEC_E_OK == ret) {
      m_ssp_info = pinfo.PackageInfo;
    } else
      DBUG_PRINT("error",
                 ("Could not obtain SSP info from authentication context"
                  ", QueryContextAttributes() failed with error %X",
                  ret));
  }

  return m_ssp_info ? m_ssp_info->Name : NULL;
}

#endif

/**
  Process result of @c {Initialize,Accept}SecurityContext() function.

  @param[in]  ret   return code from @c {Initialize,Accept}SecurityContext()
                    function

  This function analyses return value of Windows
  @c {Initialize,Accept}SecurityContext() function. A call to
  @c CompleteAuthToken() is done if requested. If authentication is complete,
  this fact is marked in the internal state of the Handshake object.
  If errors are detected the object is moved to error state.

  @return True if error has been detected.
*/

bool Handshake::process_result(int ret) {
  /*
    First check for errors and set the m_complete flag if the result
    indicates that handshake is complete.
  */

  switch (ret) {
    case SEC_E_OK:
    case SEC_I_COMPLETE_NEEDED:
      // Handshake completed
      m_complete = true;
      break;

    case SEC_I_CONTINUE_NEEDED:
    case SEC_I_COMPLETE_AND_CONTINUE:
      break;

    default:
      m_error = ret;
      return true;
  }

  m_have_sec_context = true;

  /*
    If the result indicates a need for this, complete the authentication
    token.
  */

  switch (ret) {
    case SEC_I_COMPLETE_NEEDED:
    case SEC_I_COMPLETE_AND_CONTINUE:
      ret = CompleteAuthToken(&m_sctx, &m_output);
      if (ret != 0) {
        DBUG_PRINT("error", ("CompleteAuthToken() failed with error %X", ret));
        m_error = ret;
        return true;
      }
    default:
      break;
  }

  return false;
}

/** Security_buffer class implementation **********************************/

Security_buffer::Security_buffer(const Blob &blob) : m_allocated(false) {
  init(blob.ptr(), blob.len());
}

Security_buffer::Security_buffer() : m_allocated(true) { init(NULL, 0); }

void Security_buffer::mem_free(void) {
  if (m_allocated && NULL != ptr()) {
    FreeContextBuffer(ptr());
    init(NULL, 0);
  }
}
