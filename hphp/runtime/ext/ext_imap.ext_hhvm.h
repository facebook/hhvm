/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
namespace HPHP {

TypedValue* fh_imap_8bit(TypedValue* _rv, Value* str) asm("_ZN4HPHP11f_imap_8bitERKNS_6StringE");

TypedValue* fh_imap_alerts(TypedValue* _rv) asm("_ZN4HPHP13f_imap_alertsEv");

bool fh_imap_append(Value* imap_stream, Value* mailbox, Value* message, Value* options) asm("_ZN4HPHP13f_imap_appendERKNS_6ObjectERKNS_6StringES5_S5_");

TypedValue* fh_imap_base64(TypedValue* _rv, Value* text) asm("_ZN4HPHP13f_imap_base64ERKNS_6StringE");

TypedValue* fh_imap_binary(TypedValue* _rv, Value* str) asm("_ZN4HPHP13f_imap_binaryERKNS_6StringE");

TypedValue* fh_imap_body(TypedValue* _rv, Value* imap_stream, long msg_number, long options) asm("_ZN4HPHP11f_imap_bodyERKNS_6ObjectEll");

TypedValue* fh_imap_bodystruct(TypedValue* _rv, Value* imap_stream, long msg_number, Value* section) asm("_ZN4HPHP17f_imap_bodystructERKNS_6ObjectElRKNS_6StringE");

TypedValue* fh_imap_check(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP12f_imap_checkERKNS_6ObjectE");

bool fh_imap_clearflag_full(Value* imap_stream, Value* sequence, Value* flag, long options) asm("_ZN4HPHP21f_imap_clearflag_fullERKNS_6ObjectERKNS_6StringES5_l");

bool fh_imap_close(Value* imap_stream, long flag) asm("_ZN4HPHP12f_imap_closeERKNS_6ObjectEl");

bool fh_imap_createmailbox(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP20f_imap_createmailboxERKNS_6ObjectERKNS_6StringE");

bool fh_imap_delete(Value* imap_stream, Value* msg_number, long options) asm("_ZN4HPHP13f_imap_deleteERKNS_6ObjectERKNS_6StringEl");

bool fh_imap_deletemailbox(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP20f_imap_deletemailboxERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_imap_errors(TypedValue* _rv) asm("_ZN4HPHP13f_imap_errorsEv");

bool fh_imap_expunge(Value* imap_stream) asm("_ZN4HPHP14f_imap_expungeERKNS_6ObjectE");

TypedValue* fh_imap_fetch_overview(TypedValue* _rv, Value* imap_stream, Value* sequence, long options) asm("_ZN4HPHP21f_imap_fetch_overviewERKNS_6ObjectERKNS_6StringEl");

TypedValue* fh_imap_fetchbody(TypedValue* _rv, Value* imap_stream, long msg_number, Value* section, long options) asm("_ZN4HPHP16f_imap_fetchbodyERKNS_6ObjectElRKNS_6StringEl");

TypedValue* fh_imap_fetchheader(TypedValue* _rv, Value* imap_stream, long msg_number, long options) asm("_ZN4HPHP18f_imap_fetchheaderERKNS_6ObjectEll");

TypedValue* fh_imap_fetchstructure(TypedValue* _rv, Value* imap_stream, long msg_number, long options) asm("_ZN4HPHP21f_imap_fetchstructureERKNS_6ObjectEll");

bool fh_imap_gc(Value* imap_stream, long caches) asm("_ZN4HPHP9f_imap_gcERKNS_6ObjectEl");

TypedValue* fh_imap_get_quota(TypedValue* _rv, Value* imap_stream, Value* quota_root) asm("_ZN4HPHP16f_imap_get_quotaERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_imap_get_quotaroot(TypedValue* _rv, Value* imap_stream, Value* quota_root) asm("_ZN4HPHP20f_imap_get_quotarootERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_imap_getacl(TypedValue* _rv, Value* imap_stream, Value* mailbox) asm("_ZN4HPHP13f_imap_getaclERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_imap_getmailboxes(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP19f_imap_getmailboxesERKNS_6ObjectERKNS_6StringES5_");

TypedValue* fh_imap_getsubscribed(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP20f_imap_getsubscribedERKNS_6ObjectERKNS_6StringES5_");

TypedValue* fh_imap_header(TypedValue* _rv, Value* imap_stream, long msg_number, long fromlength, long subjectlength, Value* defaulthost) asm("_ZN4HPHP13f_imap_headerERKNS_6ObjectElllRKNS_6StringE");

TypedValue* fh_imap_headerinfo(TypedValue* _rv, Value* imap_stream, long msg_number, long fromlength, long subjectlength, Value* defaulthost) asm("_ZN4HPHP17f_imap_headerinfoERKNS_6ObjectElllRKNS_6StringE");

TypedValue* fh_imap_headers(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP14f_imap_headersERKNS_6ObjectE");

TypedValue* fh_imap_last_error(TypedValue* _rv) asm("_ZN4HPHP17f_imap_last_errorEv");

TypedValue* fh_imap_list(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP11f_imap_listERKNS_6ObjectERKNS_6StringES5_");

TypedValue* fh_imap_listmailbox(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP18f_imap_listmailboxERKNS_6ObjectERKNS_6StringES5_");

TypedValue* fh_imap_listscan(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern, Value* content) asm("_ZN4HPHP15f_imap_listscanERKNS_6ObjectERKNS_6StringES5_S5_");

TypedValue* fh_imap_listsubscribed(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP21f_imap_listsubscribedERKNS_6ObjectERKNS_6StringES5_");

TypedValue* fh_imap_lsub(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP11f_imap_lsubERKNS_6ObjectERKNS_6StringES5_");

TypedValue* fh_imap_mail_compose(TypedValue* _rv, Value* envelope, Value* body) asm("_ZN4HPHP19f_imap_mail_composeERKNS_5ArrayES2_");

bool fh_imap_mail_copy(Value* imap_stream, Value* msglist, Value* mailbox, long options) asm("_ZN4HPHP16f_imap_mail_copyERKNS_6ObjectERKNS_6StringES5_l");

bool fh_imap_mail_move(Value* imap_stream, Value* msglist, Value* mailbox, long options) asm("_ZN4HPHP16f_imap_mail_moveERKNS_6ObjectERKNS_6StringES5_l");

bool fh_imap_mail(Value* to, Value* subject, Value* message, Value* additional_headers, Value* cc, Value* bcc, Value* rpath) asm("_ZN4HPHP11f_imap_mailERKNS_6StringES2_S2_S2_S2_S2_S2_");

TypedValue* fh_imap_mailboxmsginfo(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP21f_imap_mailboxmsginfoERKNS_6ObjectE");

TypedValue* fh_imap_mime_header_decode(TypedValue* _rv, Value* text) asm("_ZN4HPHP25f_imap_mime_header_decodeERKNS_6StringE");

TypedValue* fh_imap_msgno(TypedValue* _rv, Value* imap_stream, long uid) asm("_ZN4HPHP12f_imap_msgnoERKNS_6ObjectEl");

TypedValue* fh_imap_num_msg(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP14f_imap_num_msgERKNS_6ObjectE");

TypedValue* fh_imap_num_recent(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP17f_imap_num_recentERKNS_6ObjectE");

TypedValue* fh_imap_open(TypedValue* _rv, Value* mailbox, Value* username, Value* password, long options, long retries) asm("_ZN4HPHP11f_imap_openERKNS_6StringES2_S2_ll");

bool fh_imap_ping(Value* imap_stream) asm("_ZN4HPHP11f_imap_pingERKNS_6ObjectE");

TypedValue* fh_imap_qprint(TypedValue* _rv, Value* str) asm("_ZN4HPHP13f_imap_qprintERKNS_6StringE");

bool fh_imap_renamemailbox(Value* imap_stream, Value* old_mbox, Value* new_mbox) asm("_ZN4HPHP20f_imap_renamemailboxERKNS_6ObjectERKNS_6StringES5_");

bool fh_imap_reopen(Value* imap_stream, Value* mailbox, long options, long retries) asm("_ZN4HPHP13f_imap_reopenERKNS_6ObjectERKNS_6StringEll");

TypedValue* fh_imap_rfc822_parse_adrlist(TypedValue* _rv, Value* address, Value* default_host) asm("_ZN4HPHP27f_imap_rfc822_parse_adrlistERKNS_6StringES2_");

TypedValue* fh_imap_rfc822_parse_headers(TypedValue* _rv, Value* headers, Value* defaulthost) asm("_ZN4HPHP27f_imap_rfc822_parse_headersERKNS_6StringES2_");

TypedValue* fh_imap_rfc822_write_address(TypedValue* _rv, Value* mailbox, Value* host, Value* personal) asm("_ZN4HPHP27f_imap_rfc822_write_addressERKNS_6StringES2_S2_");

bool fh_imap_savebody(Value* imap_stream, TypedValue* file, long msg_number, Value* part_number, long options) asm("_ZN4HPHP15f_imap_savebodyERKNS_6ObjectERKNS_7VariantElRKNS_6StringEl");

TypedValue* fh_imap_scanmailbox(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern, Value* content) asm("_ZN4HPHP18f_imap_scanmailboxERKNS_6ObjectERKNS_6StringES5_S5_");

TypedValue* fh_imap_search(TypedValue* _rv, Value* imap_stream, Value* criteria, long options, Value* charset) asm("_ZN4HPHP13f_imap_searchERKNS_6ObjectERKNS_6StringElS5_");

bool fh_imap_set_quota(Value* imap_stream, Value* quota_root, long quota_limit) asm("_ZN4HPHP16f_imap_set_quotaERKNS_6ObjectERKNS_6StringEl");

bool fh_imap_setacl(Value* imap_stream, Value* mailbox, Value* id, Value* rights) asm("_ZN4HPHP13f_imap_setaclERKNS_6ObjectERKNS_6StringES5_S5_");

bool fh_imap_setflag_full(Value* imap_stream, Value* sequence, Value* flag, long options) asm("_ZN4HPHP19f_imap_setflag_fullERKNS_6ObjectERKNS_6StringES5_l");

TypedValue* fh_imap_sort(TypedValue* _rv, Value* imap_stream, long criteria, long reverse, long options, Value* search_criteria, Value* charset) asm("_ZN4HPHP11f_imap_sortERKNS_6ObjectElllRKNS_6StringES5_");

TypedValue* fh_imap_status(TypedValue* _rv, Value* imap_stream, Value* mailbox, long options) asm("_ZN4HPHP13f_imap_statusERKNS_6ObjectERKNS_6StringEl");

bool fh_imap_subscribe(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP16f_imap_subscribeERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_imap_thread(TypedValue* _rv, Value* imap_stream, long options) asm("_ZN4HPHP13f_imap_threadERKNS_6ObjectEl");

TypedValue* fh_imap_timeout(TypedValue* _rv, long timeout_type, long timeout) asm("_ZN4HPHP14f_imap_timeoutEll");

TypedValue* fh_imap_uid(TypedValue* _rv, Value* imap_stream, long msg_number) asm("_ZN4HPHP10f_imap_uidERKNS_6ObjectEl");

bool fh_imap_undelete(Value* imap_stream, Value* msg_number, long flags) asm("_ZN4HPHP15f_imap_undeleteERKNS_6ObjectERKNS_6StringEl");

bool fh_imap_unsubscribe(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP18f_imap_unsubscribeERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_imap_utf7_decode(TypedValue* _rv, Value* text) asm("_ZN4HPHP18f_imap_utf7_decodeERKNS_6StringE");

TypedValue* fh_imap_utf7_encode(TypedValue* _rv, Value* data) asm("_ZN4HPHP18f_imap_utf7_encodeERKNS_6StringE");

TypedValue* fh_imap_utf8(TypedValue* _rv, Value* mime_encoded_text) asm("_ZN4HPHP11f_imap_utf8ERKNS_6StringE");

} // namespace HPHP
