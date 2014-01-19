/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_EXT_IMAP_H_
#define incl_HPHP_EXT_IMAP_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_imap_8bit(const String& str);
Variant f_imap_alerts();
Variant f_imap_base64(const String& text);
Variant f_imap_binary(const String& str);
Variant f_imap_body(
  CResRef imap_stream, int64_t msg_number, int64_t options = 0);
Variant f_imap_bodystruct(
  CResRef imap_stream, int64_t msg_number, const String& section);
Variant f_imap_check(CResRef imap_stream);
bool f_imap_clearflag_full(
  CResRef imap_stream, const String& sequence, const String& flag,
  int64_t options = 0);
bool f_imap_close(CResRef imap_stream, int64_t flag = 0);
bool f_imap_createmailbox(CResRef imap_stream, const String& mailbox);
bool f_imap_delete(
  CResRef imap_stream, const String& msg_number, int64_t options = 0);
bool f_imap_deletemailbox(CResRef imap_stream, const String& mailbox);
Variant f_imap_errors();
bool f_imap_expunge(CResRef imap_stream);
Variant f_imap_fetch_overview(
  CResRef imap_stream, const String& sequence, int64_t options = 0);
Variant f_imap_fetchbody(
  CResRef imap_stream, int64_t msg_number, const String& section,
  int64_t options = 0);
Variant f_imap_fetchheader(
  CResRef imap_stream, int64_t msg_number, int64_t options = 0);
Variant f_imap_fetchstructure(
  CResRef imap_stream, int64_t msg_number, int64_t options = 0);
bool f_imap_gc(CResRef imap_stream, int64_t caches);
Variant f_imap_header(
  CResRef imap_stream, int64_t msg_number, int64_t fromlength = 0,
  int64_t subjectlength = 0, const String& defaulthost = "");
Variant f_imap_headerinfo(
  CResRef imap_stream, int64_t msg_number, int64_t fromlength = 0,
  int64_t subjectlength = 0, const String& defaulthost = "");
Variant f_imap_last_error();
Variant f_imap_list(
  CResRef imap_stream, const String& ref, const String& pattern);
Variant f_imap_listmailbox(
  CResRef imap_stream, const String& ref, const String& pattern);
bool f_imap_mail_copy(
  CResRef imap_stream, const String& msglist, const String& mailbox,
  int64_t options = 0);
bool f_imap_mail_move(
  CResRef imap_stream, const String& msglist, const String& mailbox,
  int64_t options = 0);
bool f_imap_mail(
  const String& to, const String& subject, const String& message,
  const String& additional_headers = "", const String& cc = "",
  const String& bcc = "", const String& rpath = "");
Variant f_imap_mailboxmsginfo(CResRef imap_stream);
Variant f_imap_msgno(CResRef imap_stream, int64_t uid);
Variant f_imap_num_msg(CResRef imap_stream);
Variant f_imap_num_recent(CResRef imap_stream);
Variant f_imap_open(
  const String& mailbox, const String& username, const String& password,
  int64_t options = 0, int64_t retries = 0);
bool f_imap_ping(CResRef imap_stream);
Variant f_imap_qprint(const String& str);
bool f_imap_renamemailbox(
  CResRef imap_stream, const String& old_mbox, const String& new_mbox);
bool f_imap_reopen(
  CResRef imap_stream, const String& mailbox, int64_t options = 0,
  int64_t retries = 0);
Variant f_imap_search(
  CResRef imap_stream, const String& criteria, int64_t options = 0,
  const String& charset = "");
bool f_imap_setflag_full(
  CResRef imap_stream, const String& sequence, const String& flag,
  int64_t options = 0);
Variant f_imap_status(CResRef imap_stream, const String& mailbox,
                      int64_t options = 0);
bool f_imap_subscribe(CResRef imap_stream, const String& mailbox);
Variant f_imap_timeout(int64_t timeout_type, int64_t timeout = -1);
Variant f_imap_uid(CResRef imap_stream, int64_t msg_number);
bool f_imap_undelete(
  CResRef imap_stream, const String& msg_number, int64_t flags = 0);
bool f_imap_unsubscribe(CResRef imap_stream, const String& mailbox);
Variant f_imap_utf8(const String& mime_encoded_text);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_IMAP_H_
