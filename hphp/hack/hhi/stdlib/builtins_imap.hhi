<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function imap_8bit($str) { }
<<__PHPStdLib>>
function imap_alerts() { }
<<__PHPStdLib>>
function imap_append($imap_stream, $mailbox, $message, $options = "") { }
<<__PHPStdLib>>
function imap_base64($text) { }
<<__PHPStdLib>>
function imap_binary($str) { }
<<__PHPStdLib>>
function imap_body($imap_stream, $msg_number, $options = 0) { }
<<__PHPStdLib>>
function imap_bodystruct($imap_stream, $msg_number, $section) { }
<<__PHPStdLib>>
function imap_check($imap_stream) { }
<<__PHPStdLib>>
function imap_clearflag_full($imap_stream, $sequence, $flag, $options = 0) { }
<<__PHPStdLib>>
function imap_close($imap_stream, $flag = 0) { }
<<__PHPStdLib>>
function imap_createmailbox($imap_stream, $mailbox) { }
<<__PHPStdLib>>
function imap_delete($imap_stream, $msg_number, $options = 0) { }
<<__PHPStdLib>>
function imap_deletemailbox($imap_stream, $mailbox) { }
<<__PHPStdLib>>
function imap_errors() { }
<<__PHPStdLib>>
function imap_expunge($imap_stream) { }
<<__PHPStdLib>>
function imap_fetch_overview($imap_stream, $sequence, $options = 0) { }
<<__PHPStdLib>>
function imap_fetchbody($imap_stream, $msg_number, $section, $options = 0) { }
<<__PHPStdLib>>
function imap_fetchheader($imap_stream, $msg_number, $options = 0) { }
<<__PHPStdLib>>
function imap_fetchstructure($imap_stream, $msg_number, $options = 0) { }
<<__PHPStdLib>>
function imap_gc($imap_stream, $caches) { }
<<__PHPStdLib>>
function imap_get_quota($imap_stream, $quota_root) { }
<<__PHPStdLib>>
function imap_get_quotaroot($imap_stream, $quota_root) { }
<<__PHPStdLib>>
function imap_getacl($imap_stream, $mailbox) { }
<<__PHPStdLib>>
function imap_getmailboxes($imap_stream, $ref, $pattern) { }
<<__PHPStdLib>>
function imap_getsubscribed($imap_stream, $ref, $pattern) { }
<<__PHPStdLib>>
function imap_header($imap_stream, $msg_number, $fromlength = 0, $subjectlength = 0, $defaulthost = "") { }
<<__PHPStdLib>>
function imap_headerinfo($imap_stream, $msg_number, $fromlength = 0, $subjectlength = 0, $defaulthost = "") { }
<<__PHPStdLib>>
function imap_headers($imap_stream) { }
<<__PHPStdLib>>
function imap_last_error() { }
<<__PHPStdLib>>
function imap_list($imap_stream, $ref, $pattern) { }
<<__PHPStdLib>>
function imap_listmailbox($imap_stream, $ref, $pattern) { }
<<__PHPStdLib>>
function imap_listscan($imap_stream, $ref, $pattern, $content) { }
<<__PHPStdLib>>
function imap_listsubscribed($imap_stream, $ref, $pattern) { }
<<__PHPStdLib>>
function imap_lsub($imap_stream, $ref, $pattern) { }
<<__PHPStdLib>>
function imap_mail_compose($envelope, $body) { }
<<__PHPStdLib>>
function imap_mail_copy($imap_stream, $msglist, $mailbox, $options = 0) { }
<<__PHPStdLib>>
function imap_mail_move($imap_stream, $msglist, $mailbox, $options = 0) { }
<<__PHPStdLib>>
function imap_mail($to, $subject, $message, $additional_headers = "", $cc = "", $bcc = "", $rpath = "") { }
<<__PHPStdLib>>
function imap_mailboxmsginfo($imap_stream) { }
<<__PHPStdLib>>
function imap_mime_header_decode($text) { }
<<__PHPStdLib>>
function imap_msgno($imap_stream, $uid) { }
<<__PHPStdLib>>
function imap_num_msg($imap_stream) { }
<<__PHPStdLib>>
function imap_num_recent($imap_stream) { }
<<__PHPStdLib>>
function imap_open($mailbox, $username, $password, $options = 0, $retries = 0) { }
<<__PHPStdLib>>
function imap_ping($imap_stream) { }
<<__PHPStdLib>>
function imap_qprint($str) { }
<<__PHPStdLib>>
function imap_renamemailbox($imap_stream, $old_mbox, $new_mbox) { }
<<__PHPStdLib>>
function imap_reopen($imap_stream, $mailbox, $options = 0, $retries = 0) { }
<<__PHPStdLib>>
function imap_rfc822_parse_adrlist($address, $default_host) { }
<<__PHPStdLib>>
function imap_rfc822_parse_headers($headers, $defaulthost = "") { }
<<__PHPStdLib>>
function imap_rfc822_write_address($mailbox, $host, $personal) { }
<<__PHPStdLib>>
function imap_savebody($imap_stream, $file, $msg_number, $part_number = "", $options = 0) { }
<<__PHPStdLib>>
function imap_scanmailbox($imap_stream, $ref, $pattern, $content) { }
<<__PHPStdLib>>
function imap_search($imap_stream, $criteria, $options = 0, $charset = "") { }
<<__PHPStdLib>>
function imap_set_quota($imap_stream, $quota_root, $quota_limit) { }
<<__PHPStdLib>>
function imap_setacl($imap_stream, $mailbox, $id, $rights) { }
<<__PHPStdLib>>
function imap_setflag_full($imap_stream, $sequence, $flag, $options = 0) { }
<<__PHPStdLib>>
function imap_sort($imap_stream, $criteria, $reverse, $options = 0, $search_criteria = "", $charset = "") { }
<<__PHPStdLib>>
function imap_status($imap_stream, $mailbox, $options = 0) { }
<<__PHPStdLib>>
function imap_subscribe($imap_stream, $mailbox) { }
<<__PHPStdLib>>
function imap_thread($imap_stream, $options = 0) { }
<<__PHPStdLib>>
function imap_timeout($timeout_type, $timeout = -1) { }
<<__PHPStdLib>>
function imap_uid($imap_stream, $msg_number) { }
<<__PHPStdLib>>
function imap_undelete($imap_stream, $msg_number, $flags = 0) { }
<<__PHPStdLib>>
function imap_unsubscribe($imap_stream, $mailbox) { }
<<__PHPStdLib>>
function imap_utf7_decode($text) { }
<<__PHPStdLib>>
function imap_utf7_encode($data) { }
<<__PHPStdLib>>
function imap_utf8($mime_encoded_text) { }
