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

/*
HPHP::Variant HPHP::f_imap_8bit(HPHP::String const&)
_ZN4HPHP11f_imap_8bitERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_imap_8bit(TypedValue* _rv, Value* str) asm("_ZN4HPHP11f_imap_8bitERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_alerts()
_ZN4HPHP13f_imap_alertsEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_imap_alerts(TypedValue* _rv) asm("_ZN4HPHP13f_imap_alertsEv");

/*
bool HPHP::f_imap_append(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP13f_imap_appendERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
imap_stream => rdi
mailbox => rsi
message => rdx
options => rcx
*/

bool fh_imap_append(Value* imap_stream, Value* mailbox, Value* message, Value* options) asm("_ZN4HPHP13f_imap_appendERKNS_6ObjectERKNS_6StringES5_S5_");

/*
HPHP::Variant HPHP::f_imap_base64(HPHP::String const&)
_ZN4HPHP13f_imap_base64ERKNS_6StringE

(return value) => rax
_rv => rdi
text => rsi
*/

TypedValue* fh_imap_base64(TypedValue* _rv, Value* text) asm("_ZN4HPHP13f_imap_base64ERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_binary(HPHP::String const&)
_ZN4HPHP13f_imap_binaryERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_imap_binary(TypedValue* _rv, Value* str) asm("_ZN4HPHP13f_imap_binaryERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_body(HPHP::Object const&, long long, long long)
_ZN4HPHP11f_imap_bodyERKNS_6ObjectExx

(return value) => rax
_rv => rdi
imap_stream => rsi
msg_number => rdx
options => rcx
*/

TypedValue* fh_imap_body(TypedValue* _rv, Value* imap_stream, long long msg_number, long long options) asm("_ZN4HPHP11f_imap_bodyERKNS_6ObjectExx");

/*
HPHP::Variant HPHP::f_imap_bodystruct(HPHP::Object const&, long long, HPHP::String const&)
_ZN4HPHP17f_imap_bodystructERKNS_6ObjectExRKNS_6StringE

(return value) => rax
_rv => rdi
imap_stream => rsi
msg_number => rdx
section => rcx
*/

TypedValue* fh_imap_bodystruct(TypedValue* _rv, Value* imap_stream, long long msg_number, Value* section) asm("_ZN4HPHP17f_imap_bodystructERKNS_6ObjectExRKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_check(HPHP::Object const&)
_ZN4HPHP12f_imap_checkERKNS_6ObjectE

(return value) => rax
_rv => rdi
imap_stream => rsi
*/

TypedValue* fh_imap_check(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP12f_imap_checkERKNS_6ObjectE");

/*
bool HPHP::f_imap_clearflag_full(HPHP::Object const&, HPHP::String const&, HPHP::String const&, long long)
_ZN4HPHP21f_imap_clearflag_fullERKNS_6ObjectERKNS_6StringES5_x

(return value) => rax
imap_stream => rdi
sequence => rsi
flag => rdx
options => rcx
*/

bool fh_imap_clearflag_full(Value* imap_stream, Value* sequence, Value* flag, long long options) asm("_ZN4HPHP21f_imap_clearflag_fullERKNS_6ObjectERKNS_6StringES5_x");

/*
bool HPHP::f_imap_close(HPHP::Object const&, long long)
_ZN4HPHP12f_imap_closeERKNS_6ObjectEx

(return value) => rax
imap_stream => rdi
flag => rsi
*/

bool fh_imap_close(Value* imap_stream, long long flag) asm("_ZN4HPHP12f_imap_closeERKNS_6ObjectEx");

/*
bool HPHP::f_imap_createmailbox(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP20f_imap_createmailboxERKNS_6ObjectERKNS_6StringE

(return value) => rax
imap_stream => rdi
mailbox => rsi
*/

bool fh_imap_createmailbox(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP20f_imap_createmailboxERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_imap_delete(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP13f_imap_deleteERKNS_6ObjectERKNS_6StringEx

(return value) => rax
imap_stream => rdi
msg_number => rsi
options => rdx
*/

bool fh_imap_delete(Value* imap_stream, Value* msg_number, long long options) asm("_ZN4HPHP13f_imap_deleteERKNS_6ObjectERKNS_6StringEx");

/*
bool HPHP::f_imap_deletemailbox(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP20f_imap_deletemailboxERKNS_6ObjectERKNS_6StringE

(return value) => rax
imap_stream => rdi
mailbox => rsi
*/

bool fh_imap_deletemailbox(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP20f_imap_deletemailboxERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_errors()
_ZN4HPHP13f_imap_errorsEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_imap_errors(TypedValue* _rv) asm("_ZN4HPHP13f_imap_errorsEv");

/*
bool HPHP::f_imap_expunge(HPHP::Object const&)
_ZN4HPHP14f_imap_expungeERKNS_6ObjectE

(return value) => rax
imap_stream => rdi
*/

bool fh_imap_expunge(Value* imap_stream) asm("_ZN4HPHP14f_imap_expungeERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_imap_fetch_overview(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP21f_imap_fetch_overviewERKNS_6ObjectERKNS_6StringEx

(return value) => rax
_rv => rdi
imap_stream => rsi
sequence => rdx
options => rcx
*/

TypedValue* fh_imap_fetch_overview(TypedValue* _rv, Value* imap_stream, Value* sequence, long long options) asm("_ZN4HPHP21f_imap_fetch_overviewERKNS_6ObjectERKNS_6StringEx");

/*
HPHP::Variant HPHP::f_imap_fetchbody(HPHP::Object const&, long long, HPHP::String const&, long long)
_ZN4HPHP16f_imap_fetchbodyERKNS_6ObjectExRKNS_6StringEx

(return value) => rax
_rv => rdi
imap_stream => rsi
msg_number => rdx
section => rcx
options => r8
*/

TypedValue* fh_imap_fetchbody(TypedValue* _rv, Value* imap_stream, long long msg_number, Value* section, long long options) asm("_ZN4HPHP16f_imap_fetchbodyERKNS_6ObjectExRKNS_6StringEx");

/*
HPHP::Variant HPHP::f_imap_fetchheader(HPHP::Object const&, long long, long long)
_ZN4HPHP18f_imap_fetchheaderERKNS_6ObjectExx

(return value) => rax
_rv => rdi
imap_stream => rsi
msg_number => rdx
options => rcx
*/

TypedValue* fh_imap_fetchheader(TypedValue* _rv, Value* imap_stream, long long msg_number, long long options) asm("_ZN4HPHP18f_imap_fetchheaderERKNS_6ObjectExx");

/*
HPHP::Variant HPHP::f_imap_fetchstructure(HPHP::Object const&, long long, long long)
_ZN4HPHP21f_imap_fetchstructureERKNS_6ObjectExx

(return value) => rax
_rv => rdi
imap_stream => rsi
msg_number => rdx
options => rcx
*/

TypedValue* fh_imap_fetchstructure(TypedValue* _rv, Value* imap_stream, long long msg_number, long long options) asm("_ZN4HPHP21f_imap_fetchstructureERKNS_6ObjectExx");

/*
bool HPHP::f_imap_gc(HPHP::Object const&, long long)
_ZN4HPHP9f_imap_gcERKNS_6ObjectEx

(return value) => rax
imap_stream => rdi
caches => rsi
*/

bool fh_imap_gc(Value* imap_stream, long long caches) asm("_ZN4HPHP9f_imap_gcERKNS_6ObjectEx");

/*
HPHP::Variant HPHP::f_imap_get_quota(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP16f_imap_get_quotaERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
imap_stream => rsi
quota_root => rdx
*/

TypedValue* fh_imap_get_quota(TypedValue* _rv, Value* imap_stream, Value* quota_root) asm("_ZN4HPHP16f_imap_get_quotaERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_get_quotaroot(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP20f_imap_get_quotarootERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
imap_stream => rsi
quota_root => rdx
*/

TypedValue* fh_imap_get_quotaroot(TypedValue* _rv, Value* imap_stream, Value* quota_root) asm("_ZN4HPHP20f_imap_get_quotarootERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_getacl(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_imap_getaclERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
imap_stream => rsi
mailbox => rdx
*/

TypedValue* fh_imap_getacl(TypedValue* _rv, Value* imap_stream, Value* mailbox) asm("_ZN4HPHP13f_imap_getaclERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_getmailboxes(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP19f_imap_getmailboxesERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
_rv => rdi
imap_stream => rsi
ref => rdx
pattern => rcx
*/

TypedValue* fh_imap_getmailboxes(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP19f_imap_getmailboxesERKNS_6ObjectERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_imap_getsubscribed(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_imap_getsubscribedERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
_rv => rdi
imap_stream => rsi
ref => rdx
pattern => rcx
*/

TypedValue* fh_imap_getsubscribed(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP20f_imap_getsubscribedERKNS_6ObjectERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_imap_header(HPHP::Object const&, long long, long long, long long, HPHP::String const&)
_ZN4HPHP13f_imap_headerERKNS_6ObjectExxxRKNS_6StringE

(return value) => rax
_rv => rdi
imap_stream => rsi
msg_number => rdx
fromlength => rcx
subjectlength => r8
defaulthost => r9
*/

TypedValue* fh_imap_header(TypedValue* _rv, Value* imap_stream, long long msg_number, long long fromlength, long long subjectlength, Value* defaulthost) asm("_ZN4HPHP13f_imap_headerERKNS_6ObjectExxxRKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_headerinfo(HPHP::Object const&, long long, long long, long long, HPHP::String const&)
_ZN4HPHP17f_imap_headerinfoERKNS_6ObjectExxxRKNS_6StringE

(return value) => rax
_rv => rdi
imap_stream => rsi
msg_number => rdx
fromlength => rcx
subjectlength => r8
defaulthost => r9
*/

TypedValue* fh_imap_headerinfo(TypedValue* _rv, Value* imap_stream, long long msg_number, long long fromlength, long long subjectlength, Value* defaulthost) asm("_ZN4HPHP17f_imap_headerinfoERKNS_6ObjectExxxRKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_headers(HPHP::Object const&)
_ZN4HPHP14f_imap_headersERKNS_6ObjectE

(return value) => rax
_rv => rdi
imap_stream => rsi
*/

TypedValue* fh_imap_headers(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP14f_imap_headersERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_imap_last_error()
_ZN4HPHP17f_imap_last_errorEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_imap_last_error(TypedValue* _rv) asm("_ZN4HPHP17f_imap_last_errorEv");

/*
HPHP::Variant HPHP::f_imap_list(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_imap_listERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
_rv => rdi
imap_stream => rsi
ref => rdx
pattern => rcx
*/

TypedValue* fh_imap_list(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP11f_imap_listERKNS_6ObjectERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_imap_listmailbox(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP18f_imap_listmailboxERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
_rv => rdi
imap_stream => rsi
ref => rdx
pattern => rcx
*/

TypedValue* fh_imap_listmailbox(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP18f_imap_listmailboxERKNS_6ObjectERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_imap_listscan(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_imap_listscanERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
imap_stream => rsi
ref => rdx
pattern => rcx
content => r8
*/

TypedValue* fh_imap_listscan(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern, Value* content) asm("_ZN4HPHP15f_imap_listscanERKNS_6ObjectERKNS_6StringES5_S5_");

/*
HPHP::Variant HPHP::f_imap_listsubscribed(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_imap_listsubscribedERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
_rv => rdi
imap_stream => rsi
ref => rdx
pattern => rcx
*/

TypedValue* fh_imap_listsubscribed(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP21f_imap_listsubscribedERKNS_6ObjectERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_imap_lsub(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_imap_lsubERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
_rv => rdi
imap_stream => rsi
ref => rdx
pattern => rcx
*/

TypedValue* fh_imap_lsub(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP11f_imap_lsubERKNS_6ObjectERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_imap_mail_compose(HPHP::Array const&, HPHP::Array const&)
_ZN4HPHP19f_imap_mail_composeERKNS_5ArrayES2_

(return value) => rax
_rv => rdi
envelope => rsi
body => rdx
*/

TypedValue* fh_imap_mail_compose(TypedValue* _rv, Value* envelope, Value* body) asm("_ZN4HPHP19f_imap_mail_composeERKNS_5ArrayES2_");

/*
bool HPHP::f_imap_mail_copy(HPHP::Object const&, HPHP::String const&, HPHP::String const&, long long)
_ZN4HPHP16f_imap_mail_copyERKNS_6ObjectERKNS_6StringES5_x

(return value) => rax
imap_stream => rdi
msglist => rsi
mailbox => rdx
options => rcx
*/

bool fh_imap_mail_copy(Value* imap_stream, Value* msglist, Value* mailbox, long long options) asm("_ZN4HPHP16f_imap_mail_copyERKNS_6ObjectERKNS_6StringES5_x");

/*
bool HPHP::f_imap_mail_move(HPHP::Object const&, HPHP::String const&, HPHP::String const&, long long)
_ZN4HPHP16f_imap_mail_moveERKNS_6ObjectERKNS_6StringES5_x

(return value) => rax
imap_stream => rdi
msglist => rsi
mailbox => rdx
options => rcx
*/

bool fh_imap_mail_move(Value* imap_stream, Value* msglist, Value* mailbox, long long options) asm("_ZN4HPHP16f_imap_mail_moveERKNS_6ObjectERKNS_6StringES5_x");

/*
bool HPHP::f_imap_mail(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_imap_mailERKNS_6StringES2_S2_S2_S2_S2_S2_

(return value) => rax
to => rdi
subject => rsi
message => rdx
additional_headers => rcx
cc => r8
bcc => r9
rpath => st0
*/

bool fh_imap_mail(Value* to, Value* subject, Value* message, Value* additional_headers, Value* cc, Value* bcc, Value* rpath) asm("_ZN4HPHP11f_imap_mailERKNS_6StringES2_S2_S2_S2_S2_S2_");

/*
HPHP::Variant HPHP::f_imap_mailboxmsginfo(HPHP::Object const&)
_ZN4HPHP21f_imap_mailboxmsginfoERKNS_6ObjectE

(return value) => rax
_rv => rdi
imap_stream => rsi
*/

TypedValue* fh_imap_mailboxmsginfo(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP21f_imap_mailboxmsginfoERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_imap_mime_header_decode(HPHP::String const&)
_ZN4HPHP25f_imap_mime_header_decodeERKNS_6StringE

(return value) => rax
_rv => rdi
text => rsi
*/

TypedValue* fh_imap_mime_header_decode(TypedValue* _rv, Value* text) asm("_ZN4HPHP25f_imap_mime_header_decodeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_msgno(HPHP::Object const&, long long)
_ZN4HPHP12f_imap_msgnoERKNS_6ObjectEx

(return value) => rax
_rv => rdi
imap_stream => rsi
uid => rdx
*/

TypedValue* fh_imap_msgno(TypedValue* _rv, Value* imap_stream, long long uid) asm("_ZN4HPHP12f_imap_msgnoERKNS_6ObjectEx");

/*
HPHP::Variant HPHP::f_imap_num_msg(HPHP::Object const&)
_ZN4HPHP14f_imap_num_msgERKNS_6ObjectE

(return value) => rax
_rv => rdi
imap_stream => rsi
*/

TypedValue* fh_imap_num_msg(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP14f_imap_num_msgERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_imap_num_recent(HPHP::Object const&)
_ZN4HPHP17f_imap_num_recentERKNS_6ObjectE

(return value) => rax
_rv => rdi
imap_stream => rsi
*/

TypedValue* fh_imap_num_recent(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP17f_imap_num_recentERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_imap_open(HPHP::String const&, HPHP::String const&, HPHP::String const&, long long, long long)
_ZN4HPHP11f_imap_openERKNS_6StringES2_S2_xx

(return value) => rax
_rv => rdi
mailbox => rsi
username => rdx
password => rcx
options => r8
retries => r9
*/

TypedValue* fh_imap_open(TypedValue* _rv, Value* mailbox, Value* username, Value* password, long long options, long long retries) asm("_ZN4HPHP11f_imap_openERKNS_6StringES2_S2_xx");

/*
bool HPHP::f_imap_ping(HPHP::Object const&)
_ZN4HPHP11f_imap_pingERKNS_6ObjectE

(return value) => rax
imap_stream => rdi
*/

bool fh_imap_ping(Value* imap_stream) asm("_ZN4HPHP11f_imap_pingERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_imap_qprint(HPHP::String const&)
_ZN4HPHP13f_imap_qprintERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_imap_qprint(TypedValue* _rv, Value* str) asm("_ZN4HPHP13f_imap_qprintERKNS_6StringE");

/*
bool HPHP::f_imap_renamemailbox(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_imap_renamemailboxERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
imap_stream => rdi
old_mbox => rsi
new_mbox => rdx
*/

bool fh_imap_renamemailbox(Value* imap_stream, Value* old_mbox, Value* new_mbox) asm("_ZN4HPHP20f_imap_renamemailboxERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_imap_reopen(HPHP::Object const&, HPHP::String const&, long long, long long)
_ZN4HPHP13f_imap_reopenERKNS_6ObjectERKNS_6StringExx

(return value) => rax
imap_stream => rdi
mailbox => rsi
options => rdx
retries => rcx
*/

bool fh_imap_reopen(Value* imap_stream, Value* mailbox, long long options, long long retries) asm("_ZN4HPHP13f_imap_reopenERKNS_6ObjectERKNS_6StringExx");

/*
HPHP::Variant HPHP::f_imap_rfc822_parse_adrlist(HPHP::String const&, HPHP::String const&)
_ZN4HPHP27f_imap_rfc822_parse_adrlistERKNS_6StringES2_

(return value) => rax
_rv => rdi
address => rsi
default_host => rdx
*/

TypedValue* fh_imap_rfc822_parse_adrlist(TypedValue* _rv, Value* address, Value* default_host) asm("_ZN4HPHP27f_imap_rfc822_parse_adrlistERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_imap_rfc822_parse_headers(HPHP::String const&, HPHP::String const&)
_ZN4HPHP27f_imap_rfc822_parse_headersERKNS_6StringES2_

(return value) => rax
_rv => rdi
headers => rsi
defaulthost => rdx
*/

TypedValue* fh_imap_rfc822_parse_headers(TypedValue* _rv, Value* headers, Value* defaulthost) asm("_ZN4HPHP27f_imap_rfc822_parse_headersERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_imap_rfc822_write_address(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP27f_imap_rfc822_write_addressERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
mailbox => rsi
host => rdx
personal => rcx
*/

TypedValue* fh_imap_rfc822_write_address(TypedValue* _rv, Value* mailbox, Value* host, Value* personal) asm("_ZN4HPHP27f_imap_rfc822_write_addressERKNS_6StringES2_S2_");

/*
bool HPHP::f_imap_savebody(HPHP::Object const&, HPHP::Variant const&, long long, HPHP::String const&, long long)
_ZN4HPHP15f_imap_savebodyERKNS_6ObjectERKNS_7VariantExRKNS_6StringEx

(return value) => rax
imap_stream => rdi
file => rsi
msg_number => rdx
part_number => rcx
options => r8
*/

bool fh_imap_savebody(Value* imap_stream, TypedValue* file, long long msg_number, Value* part_number, long long options) asm("_ZN4HPHP15f_imap_savebodyERKNS_6ObjectERKNS_7VariantExRKNS_6StringEx");

/*
HPHP::Variant HPHP::f_imap_scanmailbox(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP18f_imap_scanmailboxERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
imap_stream => rsi
ref => rdx
pattern => rcx
content => r8
*/

TypedValue* fh_imap_scanmailbox(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern, Value* content) asm("_ZN4HPHP18f_imap_scanmailboxERKNS_6ObjectERKNS_6StringES5_S5_");

/*
HPHP::Variant HPHP::f_imap_search(HPHP::Object const&, HPHP::String const&, long long, HPHP::String const&)
_ZN4HPHP13f_imap_searchERKNS_6ObjectERKNS_6StringExS5_

(return value) => rax
_rv => rdi
imap_stream => rsi
criteria => rdx
options => rcx
charset => r8
*/

TypedValue* fh_imap_search(TypedValue* _rv, Value* imap_stream, Value* criteria, long long options, Value* charset) asm("_ZN4HPHP13f_imap_searchERKNS_6ObjectERKNS_6StringExS5_");

/*
bool HPHP::f_imap_set_quota(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP16f_imap_set_quotaERKNS_6ObjectERKNS_6StringEx

(return value) => rax
imap_stream => rdi
quota_root => rsi
quota_limit => rdx
*/

bool fh_imap_set_quota(Value* imap_stream, Value* quota_root, long long quota_limit) asm("_ZN4HPHP16f_imap_set_quotaERKNS_6ObjectERKNS_6StringEx");

/*
bool HPHP::f_imap_setacl(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP13f_imap_setaclERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
imap_stream => rdi
mailbox => rsi
id => rdx
rights => rcx
*/

bool fh_imap_setacl(Value* imap_stream, Value* mailbox, Value* id, Value* rights) asm("_ZN4HPHP13f_imap_setaclERKNS_6ObjectERKNS_6StringES5_S5_");

/*
bool HPHP::f_imap_setflag_full(HPHP::Object const&, HPHP::String const&, HPHP::String const&, long long)
_ZN4HPHP19f_imap_setflag_fullERKNS_6ObjectERKNS_6StringES5_x

(return value) => rax
imap_stream => rdi
sequence => rsi
flag => rdx
options => rcx
*/

bool fh_imap_setflag_full(Value* imap_stream, Value* sequence, Value* flag, long long options) asm("_ZN4HPHP19f_imap_setflag_fullERKNS_6ObjectERKNS_6StringES5_x");

/*
HPHP::Variant HPHP::f_imap_sort(HPHP::Object const&, long long, long long, long long, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_imap_sortERKNS_6ObjectExxxRKNS_6StringES5_

(return value) => rax
_rv => rdi
imap_stream => rsi
criteria => rdx
reverse => rcx
options => r8
search_criteria => r9
charset => st0
*/

TypedValue* fh_imap_sort(TypedValue* _rv, Value* imap_stream, long long criteria, long long reverse, long long options, Value* search_criteria, Value* charset) asm("_ZN4HPHP11f_imap_sortERKNS_6ObjectExxxRKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_imap_status(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP13f_imap_statusERKNS_6ObjectERKNS_6StringEx

(return value) => rax
_rv => rdi
imap_stream => rsi
mailbox => rdx
options => rcx
*/

TypedValue* fh_imap_status(TypedValue* _rv, Value* imap_stream, Value* mailbox, long long options) asm("_ZN4HPHP13f_imap_statusERKNS_6ObjectERKNS_6StringEx");

/*
bool HPHP::f_imap_subscribe(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP16f_imap_subscribeERKNS_6ObjectERKNS_6StringE

(return value) => rax
imap_stream => rdi
mailbox => rsi
*/

bool fh_imap_subscribe(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP16f_imap_subscribeERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_thread(HPHP::Object const&, long long)
_ZN4HPHP13f_imap_threadERKNS_6ObjectEx

(return value) => rax
_rv => rdi
imap_stream => rsi
options => rdx
*/

TypedValue* fh_imap_thread(TypedValue* _rv, Value* imap_stream, long long options) asm("_ZN4HPHP13f_imap_threadERKNS_6ObjectEx");

/*
HPHP::Variant HPHP::f_imap_timeout(long long, long long)
_ZN4HPHP14f_imap_timeoutExx

(return value) => rax
_rv => rdi
timeout_type => rsi
timeout => rdx
*/

TypedValue* fh_imap_timeout(TypedValue* _rv, long long timeout_type, long long timeout) asm("_ZN4HPHP14f_imap_timeoutExx");

/*
HPHP::Variant HPHP::f_imap_uid(HPHP::Object const&, long long)
_ZN4HPHP10f_imap_uidERKNS_6ObjectEx

(return value) => rax
_rv => rdi
imap_stream => rsi
msg_number => rdx
*/

TypedValue* fh_imap_uid(TypedValue* _rv, Value* imap_stream, long long msg_number) asm("_ZN4HPHP10f_imap_uidERKNS_6ObjectEx");

/*
bool HPHP::f_imap_undelete(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP15f_imap_undeleteERKNS_6ObjectERKNS_6StringEx

(return value) => rax
imap_stream => rdi
msg_number => rsi
flags => rdx
*/

bool fh_imap_undelete(Value* imap_stream, Value* msg_number, long long flags) asm("_ZN4HPHP15f_imap_undeleteERKNS_6ObjectERKNS_6StringEx");

/*
bool HPHP::f_imap_unsubscribe(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP18f_imap_unsubscribeERKNS_6ObjectERKNS_6StringE

(return value) => rax
imap_stream => rdi
mailbox => rsi
*/

bool fh_imap_unsubscribe(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP18f_imap_unsubscribeERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_utf7_decode(HPHP::String const&)
_ZN4HPHP18f_imap_utf7_decodeERKNS_6StringE

(return value) => rax
_rv => rdi
text => rsi
*/

TypedValue* fh_imap_utf7_decode(TypedValue* _rv, Value* text) asm("_ZN4HPHP18f_imap_utf7_decodeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_utf7_encode(HPHP::String const&)
_ZN4HPHP18f_imap_utf7_encodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_imap_utf7_encode(TypedValue* _rv, Value* data) asm("_ZN4HPHP18f_imap_utf7_encodeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_imap_utf8(HPHP::String const&)
_ZN4HPHP11f_imap_utf8ERKNS_6StringE

(return value) => rax
_rv => rdi
mime_encoded_text => rsi
*/

TypedValue* fh_imap_utf8(TypedValue* _rv, Value* mime_encoded_text) asm("_ZN4HPHP11f_imap_utf8ERKNS_6StringE");


} // !HPHP

