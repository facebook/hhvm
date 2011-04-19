/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <test/test_ext_imap.h>
#include <runtime/ext/ext_imap.h>

IMPLEMENT_SEP_EXTENSION_TEST(Imap);
///////////////////////////////////////////////////////////////////////////////

bool TestExtImap::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_imap_8bit);
  RUN_TEST(test_imap_alerts);
  RUN_TEST(test_imap_append);
  RUN_TEST(test_imap_base64);
  RUN_TEST(test_imap_binary);
  RUN_TEST(test_imap_body);
  RUN_TEST(test_imap_bodystruct);
  RUN_TEST(test_imap_check);
  RUN_TEST(test_imap_clearflag_full);
  RUN_TEST(test_imap_close);
  RUN_TEST(test_imap_createmailbox);
  RUN_TEST(test_imap_delete);
  RUN_TEST(test_imap_deletemailbox);
  RUN_TEST(test_imap_errors);
  RUN_TEST(test_imap_expunge);
  RUN_TEST(test_imap_fetch_overview);
  RUN_TEST(test_imap_fetchbody);
  RUN_TEST(test_imap_fetchheader);
  RUN_TEST(test_imap_fetchstructure);
  RUN_TEST(test_imap_gc);
  RUN_TEST(test_imap_get_quota);
  RUN_TEST(test_imap_get_quotaroot);
  RUN_TEST(test_imap_getacl);
  RUN_TEST(test_imap_getmailboxes);
  RUN_TEST(test_imap_getsubscribed);
  RUN_TEST(test_imap_header);
  RUN_TEST(test_imap_headerinfo);
  RUN_TEST(test_imap_headers);
  RUN_TEST(test_imap_last_error);
  RUN_TEST(test_imap_list);
  RUN_TEST(test_imap_listmailbox);
  RUN_TEST(test_imap_listscan);
  RUN_TEST(test_imap_listsubscribed);
  RUN_TEST(test_imap_lsub);
  RUN_TEST(test_imap_mail_compose);
  RUN_TEST(test_imap_mail_copy);
  RUN_TEST(test_imap_mail_move);
  RUN_TEST(test_imap_mail);
  RUN_TEST(test_imap_mailboxmsginfo);
  RUN_TEST(test_imap_mime_header_decode);
  RUN_TEST(test_imap_msgno);
  RUN_TEST(test_imap_num_msg);
  RUN_TEST(test_imap_num_recent);
  RUN_TEST(test_imap_open);
  RUN_TEST(test_imap_ping);
  RUN_TEST(test_imap_qprint);
  RUN_TEST(test_imap_renamemailbox);
  RUN_TEST(test_imap_reopen);
  RUN_TEST(test_imap_rfc822_parse_adrlist);
  RUN_TEST(test_imap_rfc822_parse_headers);
  RUN_TEST(test_imap_rfc822_write_address);
  RUN_TEST(test_imap_savebody);
  RUN_TEST(test_imap_scanmailbox);
  RUN_TEST(test_imap_search);
  RUN_TEST(test_imap_set_quota);
  RUN_TEST(test_imap_setacl);
  RUN_TEST(test_imap_setflag_full);
  RUN_TEST(test_imap_sort);
  RUN_TEST(test_imap_status);
  RUN_TEST(test_imap_subscribe);
  RUN_TEST(test_imap_thread);
  RUN_TEST(test_imap_timeout);
  RUN_TEST(test_imap_uid);
  RUN_TEST(test_imap_undelete);
  RUN_TEST(test_imap_unsubscribe);
  RUN_TEST(test_imap_utf7_decode);
  RUN_TEST(test_imap_utf7_encode);
  RUN_TEST(test_imap_utf8);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtImap::test_imap_8bit() {
  return Count(true);
}

bool TestExtImap::test_imap_alerts() {
  return Count(true);
}

bool TestExtImap::test_imap_append() {
  return Count(true);
}

bool TestExtImap::test_imap_base64() {
  return Count(true);
}

bool TestExtImap::test_imap_binary() {
  return Count(true);
}

bool TestExtImap::test_imap_body() {
  return Count(true);
}

bool TestExtImap::test_imap_bodystruct() {
  return Count(true);
}

bool TestExtImap::test_imap_check() {
  return Count(true);
}

bool TestExtImap::test_imap_clearflag_full() {
  return Count(true);
}

bool TestExtImap::test_imap_close() {
  return Count(true);
}

bool TestExtImap::test_imap_createmailbox() {
  return Count(true);
}

bool TestExtImap::test_imap_delete() {
  return Count(true);
}

bool TestExtImap::test_imap_deletemailbox() {
  return Count(true);
}

bool TestExtImap::test_imap_errors() {
  return Count(true);
}

bool TestExtImap::test_imap_expunge() {
  return Count(true);
}

bool TestExtImap::test_imap_fetch_overview() {
  return Count(true);
}

bool TestExtImap::test_imap_fetchbody() {
  return Count(true);
}

bool TestExtImap::test_imap_fetchheader() {
  return Count(true);
}

bool TestExtImap::test_imap_fetchstructure() {
  return Count(true);
}

bool TestExtImap::test_imap_gc() {
  return Count(true);
}

bool TestExtImap::test_imap_get_quota() {
  return Count(true);
}

bool TestExtImap::test_imap_get_quotaroot() {
  return Count(true);
}

bool TestExtImap::test_imap_getacl() {
  return Count(true);
}

bool TestExtImap::test_imap_getmailboxes() {
  return Count(true);
}

bool TestExtImap::test_imap_getsubscribed() {
  return Count(true);
}

bool TestExtImap::test_imap_header() {
  return Count(true);
}

bool TestExtImap::test_imap_headerinfo() {
  return Count(true);
}

bool TestExtImap::test_imap_headers() {
  return Count(true);
}

bool TestExtImap::test_imap_last_error() {
  return Count(true);
}

bool TestExtImap::test_imap_list() {
  return Count(true);
}

bool TestExtImap::test_imap_listmailbox() {
  return Count(true);
}

bool TestExtImap::test_imap_listscan() {
  return Count(true);
}

bool TestExtImap::test_imap_listsubscribed() {
  return Count(true);
}

bool TestExtImap::test_imap_lsub() {
  return Count(true);
}

bool TestExtImap::test_imap_mail_compose() {
  return Count(true);
}

bool TestExtImap::test_imap_mail_copy() {
  return Count(true);
}

bool TestExtImap::test_imap_mail_move() {
  return Count(true);
}

bool TestExtImap::test_imap_mail() {
  return Count(true);
}

bool TestExtImap::test_imap_mailboxmsginfo() {
  return Count(true);
}

bool TestExtImap::test_imap_mime_header_decode() {
  return Count(true);
}

bool TestExtImap::test_imap_msgno() {
  return Count(true);
}

bool TestExtImap::test_imap_num_msg() {
  return Count(true);
}

bool TestExtImap::test_imap_num_recent() {
  return Count(true);
}

bool TestExtImap::test_imap_open() {
  return Count(true);
}

bool TestExtImap::test_imap_ping() {
  return Count(true);
}

bool TestExtImap::test_imap_qprint() {
  return Count(true);
}

bool TestExtImap::test_imap_renamemailbox() {
  return Count(true);
}

bool TestExtImap::test_imap_reopen() {
  return Count(true);
}

bool TestExtImap::test_imap_rfc822_parse_adrlist() {
  return Count(true);
}

bool TestExtImap::test_imap_rfc822_parse_headers() {
  return Count(true);
}

bool TestExtImap::test_imap_rfc822_write_address() {
  return Count(true);
}

bool TestExtImap::test_imap_savebody() {
  return Count(true);
}

bool TestExtImap::test_imap_scanmailbox() {
  return Count(true);
}

bool TestExtImap::test_imap_search() {
  return Count(true);
}

bool TestExtImap::test_imap_set_quota() {
  return Count(true);
}

bool TestExtImap::test_imap_setacl() {
  return Count(true);
}

bool TestExtImap::test_imap_setflag_full() {
  return Count(true);
}

bool TestExtImap::test_imap_sort() {
  return Count(true);
}

bool TestExtImap::test_imap_status() {
  return Count(true);
}

bool TestExtImap::test_imap_subscribe() {
  return Count(true);
}

bool TestExtImap::test_imap_thread() {
  return Count(true);
}

bool TestExtImap::test_imap_timeout() {
  return Count(true);
}

bool TestExtImap::test_imap_uid() {
  return Count(true);
}

bool TestExtImap::test_imap_undelete() {
  return Count(true);
}

bool TestExtImap::test_imap_unsubscribe() {
  return Count(true);
}

bool TestExtImap::test_imap_utf7_decode() {
  return Count(true);
}

bool TestExtImap::test_imap_utf7_encode() {
  return Count(true);
}

bool TestExtImap::test_imap_utf8() {
  return Count(true);
}
