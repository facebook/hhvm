<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function imap_8bit($str) { }
function imap_alerts() { }
function imap_append($imap_stream, $mailbox, $message, $options = "") { }
function imap_base64($text) { }
function imap_binary($str) { }
function imap_body($imap_stream, $msg_number, $options = 0) { }
function imap_bodystruct($imap_stream, $msg_number, $section) { }
function imap_check($imap_stream) { }
function imap_clearflag_full($imap_stream, $sequence, $flag, $options = 0) { }
function imap_close($imap_stream, $flag = 0) { }
function imap_createmailbox($imap_stream, $mailbox) { }
function imap_delete($imap_stream, $msg_number, $options = 0) { }
function imap_deletemailbox($imap_stream, $mailbox) { }
function imap_errors() { }
function imap_expunge($imap_stream) { }
function imap_fetch_overview($imap_stream, $sequence, $options = 0) { }
function imap_fetchbody($imap_stream, $msg_number, $section, $options = 0) { }
function imap_fetchheader($imap_stream, $msg_number, $options = 0) { }
function imap_fetchstructure($imap_stream, $msg_number, $options = 0) { }
function imap_gc($imap_stream, $caches) { }
function imap_get_quota($imap_stream, $quota_root) { }
function imap_get_quotaroot($imap_stream, $quota_root) { }
function imap_getacl($imap_stream, $mailbox) { }
function imap_getmailboxes($imap_stream, $ref, $pattern) { }
function imap_getsubscribed($imap_stream, $ref, $pattern) { }
function imap_header($imap_stream, $msg_number, $fromlength = 0, $subjectlength = 0, $defaulthost = "") { }
function imap_headerinfo($imap_stream, $msg_number, $fromlength = 0, $subjectlength = 0, $defaulthost = "") { }
function imap_headers($imap_stream) { }
function imap_last_error() { }
function imap_list($imap_stream, $ref, $pattern) { }
function imap_listmailbox($imap_stream, $ref, $pattern) { }
function imap_listscan($imap_stream, $ref, $pattern, $content) { }
function imap_listsubscribed($imap_stream, $ref, $pattern) { }
function imap_lsub($imap_stream, $ref, $pattern) { }
function imap_mail_compose($envelope, $body) { }
function imap_mail_copy($imap_stream, $msglist, $mailbox, $options = 0) { }
function imap_mail_move($imap_stream, $msglist, $mailbox, $options = 0) { }
function imap_mail($to, $subject, $message, $additional_headers = "", $cc = "", $bcc = "", $rpath = "") { }
function imap_mailboxmsginfo($imap_stream) { }
function imap_mime_header_decode($text) { }
function imap_msgno($imap_stream, $uid) { }
function imap_num_msg($imap_stream) { }
function imap_num_recent($imap_stream) { }
function imap_open($mailbox, $username, $password, $options = 0, $retries = 0) { }
function imap_ping($imap_stream) { }
function imap_qprint($str) { }
function imap_renamemailbox($imap_stream, $old_mbox, $new_mbox) { }
function imap_reopen($imap_stream, $mailbox, $options = 0, $retries = 0) { }
function imap_rfc822_parse_adrlist($address, $default_host) { }
function imap_rfc822_parse_headers($headers, $defaulthost = "") { }
function imap_rfc822_write_address($mailbox, $host, $personal) { }
function imap_savebody($imap_stream, $file, $msg_number, $part_number = "", $options = 0) { }
function imap_scanmailbox($imap_stream, $ref, $pattern, $content) { }
function imap_search($imap_stream, $criteria, $options = 0, $charset = "") { }
function imap_set_quota($imap_stream, $quota_root, $quota_limit) { }
function imap_setacl($imap_stream, $mailbox, $id, $rights) { }
function imap_setflag_full($imap_stream, $sequence, $flag, $options = 0) { }
function imap_sort($imap_stream, $criteria, $reverse, $options = 0, $search_criteria = "", $charset = "") { }
function imap_status($imap_stream, $mailbox, $options = 0) { }
function imap_subscribe($imap_stream, $mailbox) { }
function imap_thread($imap_stream, $options = 0) { }
function imap_timeout($timeout_type, $timeout = -1) { }
function imap_uid($imap_stream, $msg_number) { }
function imap_undelete($imap_stream, $msg_number, $flags = 0) { }
function imap_unsubscribe($imap_stream, $mailbox) { }
function imap_utf7_decode($text) { }
function imap_utf7_encode($data) { }
function imap_utf8($mime_encoded_text) { }
