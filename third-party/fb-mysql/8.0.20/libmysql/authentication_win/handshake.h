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

#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include "common.h"
#include "my_inttypes.h"

/**
  Name of the SSP (Security Support Provider) to be used for authentication.

  We use "Negotiate" which will find the most secure SSP which can be used
  and redirect to that SSP.
*/
#define SSP_NAME "Negotiate"

/**
  Maximal number of rounds in authentication handshake.

  Server will interrupt authentication handshake with error if client's
  identity can not be determined within this many rounds.
*/
#define MAX_HANDSHAKE_ROUNDS 50

/// Convenience wrapper around @c SecBufferDesc.

class Security_buffer : public SecBufferDesc {
  SecBuffer m_buf;  ///< A @c SecBuffer instance.

  void init(byte *ptr, size_t len) {
    ulVersion = 0;
    cBuffers = 1;
    pBuffers = &m_buf;

    m_buf.BufferType = SECBUFFER_TOKEN;
    m_buf.pvBuffer = ptr;
    m_buf.cbBuffer = (ulong)len;
  }

  /// If @c false, no deallocation will be done in the destructor.
  const bool m_allocated;

  // Copying/assignment is not supported and can lead to memory leaks
  // So declaring copy constructor and assignment operator as private
  Security_buffer(const Security_buffer &);
  const Security_buffer &operator=(const Security_buffer &);

 public:
  Security_buffer(const Blob &);
  Security_buffer();

  ~Security_buffer() { mem_free(); }

  byte *ptr() const { return (byte *)m_buf.pvBuffer; }

  size_t len() const { return m_buf.cbBuffer; }

  const Blob as_blob() const { return Blob(ptr(), len()); }

  void mem_free(void);
};

/// Common base for Handshake_{server,client}.

class Handshake {
 public:
  typedef enum { CLIENT, SERVER } side_t;

  Handshake(const char *ssp, side_t side);
  virtual ~Handshake();

  int packet_processing_loop();

  bool virtual is_complete() const { return m_complete; }

  int error() const { return m_error; }

 protected:
  /// Security context object created during the handshake.
  CtxtHandle m_sctx;

  /// Credentials of the principal performing this handshake.
  CredHandle m_cred;

  /// Stores expiry date of the created security context.
  TimeStamp m_expire;

  /// Stores attributes of the created security context.
  ULONG m_atts;

  /**
    Round of the handshake (starting from round 1). One round
    consist of reading packet from the other side, processing it and
    optionally sending a reply (see @c packet_processing_loop()).
  */
  unsigned int m_round;

  /// If non-zero, stores error code of the last failed operation.
  int m_error;

  /// @c true when handshake is complete.
  bool m_complete;

  /// @c true when the principal credentials has been determined.
  bool m_have_credentials;

  /// @c true when the security context has been created.
  bool m_have_sec_context;

  /// Buffer for data to be send to the other side.
  Security_buffer m_output;

  bool process_result(int);

  /**
    This method is used inside @c packet_processing_loop to process
    data packets received from the other end.

    @param  data  data to be processed

    @return A blob with data to be sent to the other end or null blob if
    no more data needs to be exchanged.
  */
  virtual Blob process_data(const Blob &data) = 0;

  /// Read packet from the other end.
  virtual Blob read_packet() = 0;

  /// Write packet to the other end.
  virtual int write_packet(Blob &data) = 0;

#ifndef DBUG_OFF

 private:
  SecPkgInfo *m_ssp_info;

 public:
  const char *ssp_name();

#endif
};

#endif
