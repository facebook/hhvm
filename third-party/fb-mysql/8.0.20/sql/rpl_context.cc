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

#include "sql/rpl_context.h"

#include <stddef.h>

#include "libbinlogevents/include/compression/factory.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_sqlcommand.h"
#include "sql/binlog_ostream.h"
#include "sql/rpl_gtid.h"   // Gtid_set
#include "sql/sql_class.h"  // THD
#include "sql/sql_lex.h"
#include "sql/system_variables.h"

Session_consistency_gtids_ctx::Session_consistency_gtids_ctx()
    : m_sid_map(nullptr),
      m_gtid_set(nullptr),
      m_listener(nullptr),
      m_curr_session_track_gtids(OFF) {}

Session_consistency_gtids_ctx::~Session_consistency_gtids_ctx() {
  if (m_gtid_set) {
    delete m_gtid_set;
    m_gtid_set = nullptr;
  }

  if (m_sid_map) {
    delete m_sid_map;
    m_sid_map = nullptr;
  }
}

inline bool Session_consistency_gtids_ctx::shall_collect(const THD *thd) {
  return /* Do not track OWN_GTID if session does not own a
            (non-anonymous) GTID. */
      (thd->owned_gtid.sidno > 0 || m_curr_session_track_gtids == ALL_GTIDS) &&
      /* if there is no listener/tracker, then there is no reason to collect */
      m_listener != nullptr &&
      /* No need to track GTIDs for system threads to avoid performance issues
         like replication lag */
      thd->system_thread == NON_SYSTEM_THREAD &&
      /* ROLLBACK statements may end up calling trans_commit_stmt */
      thd->lex->sql_command != SQLCOM_ROLLBACK &&
      thd->lex->sql_command != SQLCOM_ROLLBACK_TO_SAVEPOINT;
}

bool Session_consistency_gtids_ctx::notify_after_transaction_commit(
    const THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  bool res = false;

  if (!shall_collect(thd)) return res;

  if (m_curr_session_track_gtids == ALL_GTIDS) {
    /*
     If one is configured to read all writes, we always collect
     the GTID_EXECUTED.

     NOTE: in the future optimize to collect deltas instead maybe.
    */
    global_sid_lock->wrlock();
    res = m_gtid_set->add_gtid_set(gtid_state->get_executed_gtids()) !=
          RETURN_STATUS_OK;
    global_sid_lock->unlock();

    if (!res) notify_ctx_change_listener();
  }

  return res;
}

bool Session_consistency_gtids_ctx::notify_after_gtid_executed_update(
    const THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  bool res = false;

  if (!shall_collect(thd)) return res;

  if (m_curr_session_track_gtids == OWN_GTID) {
    DBUG_ASSERT(get_gtid_mode(GTID_MODE_LOCK_SID) != GTID_MODE_OFF);
    DBUG_ASSERT(thd->owned_gtid.sidno > 0);
    const Gtid &gtid = thd->owned_gtid;
    if (gtid.sidno == -1)  // we need to add thd->owned_gtid_set
    {
      /* Caller must only call this function if the set was not empty. */
#ifdef HAVE_GTID_NEXT_LIST
      DBUG_ASSERT(!thd->owned_gtid_set.is_empty());
      res = m_gtid_set->add_gtid_set(&thd->owned_gtid_set) != RETURN_STATUS_OK;
#else
      DBUG_ASSERT(0);
#endif
    } else if (gtid.sidno > 0)  // only one gtid
    {
      /*
        Note that the interface is such that m_sid_map must contain
        sidno before we add the gtid to m_gtid_set.

        Thus, to avoid relying on global_sid_map and thus contributing
        to increased contention, we arrange for sidnos on the local
        sid map.
      */
      rpl_sidno local_set_sidno = m_sid_map->add_sid(thd->owned_sid);

      DBUG_ASSERT(!m_gtid_set->contains_gtid(local_set_sidno, gtid.gno));
      res = m_gtid_set->ensure_sidno(local_set_sidno) != RETURN_STATUS_OK;
      if (!res) m_gtid_set->_add_gtid(local_set_sidno, gtid.gno);
    }

    if (!res) notify_ctx_change_listener();
  }
  return res;
}

bool Session_consistency_gtids_ctx::notify_after_response_packet(
    const THD *thd) {
  int res = false;
  DBUG_TRACE;

  if (m_gtid_set && !m_gtid_set->is_empty()) m_gtid_set->clear();

  /*
   Every time we get a notification that a packet was sent, we update
   this value. It may have changed (the previous command may have been
   a SET SESSION session_track_gtids=...;).
   */
  m_curr_session_track_gtids = thd->variables.session_track_gtids;
  return res;
}

void Session_consistency_gtids_ctx::register_ctx_change_listener(
    Session_consistency_gtids_ctx::Ctx_change_listener *listener, THD *thd) {
  DBUG_ASSERT(m_listener == nullptr || m_listener == listener);
  if (m_listener == nullptr) {
    DBUG_ASSERT(m_sid_map == nullptr && m_gtid_set == nullptr);
    m_listener = listener;
    m_sid_map = new Sid_map(nullptr);
    m_gtid_set = new Gtid_set(m_sid_map);

    /*
     Caches the value at startup if needed. This is called during THD::init,
     if the session_track_gtids value is set at startup time to anything
     different than OFF.
     */
    m_curr_session_track_gtids = thd->variables.session_track_gtids;
  }
}

void Session_consistency_gtids_ctx::unregister_ctx_change_listener(
    Session_consistency_gtids_ctx::Ctx_change_listener *listener
        MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(m_listener == listener || m_listener == nullptr);

  if (m_gtid_set) delete m_gtid_set;

  if (m_sid_map) delete m_sid_map;

  m_listener = nullptr;
  m_gtid_set = nullptr;
  m_sid_map = nullptr;
}

Last_used_gtid_tracker_ctx::Last_used_gtid_tracker_ctx() {
  m_last_used_gtid = std::unique_ptr<Gtid>(new Gtid{0, 0});
}

Last_used_gtid_tracker_ctx::~Last_used_gtid_tracker_ctx() {}

void Last_used_gtid_tracker_ctx::set_last_used_gtid(const Gtid &gtid) {
  (*m_last_used_gtid).set(gtid.sidno, gtid.gno);
}

void Last_used_gtid_tracker_ctx::get_last_used_gtid(Gtid &gtid) {
  gtid.sidno = (*m_last_used_gtid).sidno;
  gtid.gno = (*m_last_used_gtid).gno;
}

const size_t Transaction_compression_ctx::DEFAULT_COMPRESSION_BUFFER_SIZE =
    1024;

Transaction_compression_ctx::Transaction_compression_ctx()
    : m_compressor(nullptr) {}

Transaction_compression_ctx::~Transaction_compression_ctx() {
  if (m_compressor) {
    unsigned char *buffer = nullptr;
    std::tie(buffer, std::ignore, std::ignore) = m_compressor->get_buffer();
    delete m_compressor;
    if (buffer) free(buffer);
  }
}

binary_log::transaction::compression::Compressor *
Transaction_compression_ctx::get_compressor(THD *thd) {
  auto ctype = (binary_log::transaction::compression::type)
                   thd->variables.binlog_trx_compression_type;

  if (m_compressor == nullptr ||
      (m_compressor->compression_type_code() != ctype)) {
    unsigned char *buffer = nullptr;
    std::size_t capacity = 0;

    // delete the existing one, reuse the buffer
    if (m_compressor) {
      std::tie(buffer, std::ignore, capacity) = m_compressor->get_buffer();
      delete m_compressor;
      m_compressor = nullptr;
    }

    // TODO: consider moving m_decompressor to a shared_ptr
    auto comp =
        binary_log::transaction::compression::Factory::build_compressor(ctype);

    if (comp != nullptr) {
      m_compressor = comp.release();

      // inject an output buffer if possible, if not, then delete the compressor
      if (buffer == nullptr)
        buffer = (unsigned char *)malloc(DEFAULT_COMPRESSION_BUFFER_SIZE);

      if (buffer != nullptr)
        m_compressor->set_buffer(buffer, DEFAULT_COMPRESSION_BUFFER_SIZE);
      else {
        /* purecov: begin inspected */
        // OOM
        delete m_compressor;
        m_compressor = nullptr;
        /* purecov: end */
      }
    }
  }
  return m_compressor;
}
