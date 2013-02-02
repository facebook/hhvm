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
long long HPHP::f_ftok(HPHP::String const&, HPHP::String const&)
_ZN4HPHP6f_ftokERKNS_6StringES2_

(return value) => rax
pathname => rdi
proj => rsi
*/

long long fh_ftok(Value* pathname, Value* proj) asm("_ZN4HPHP6f_ftokERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_msg_get_queue(long long, long long)
_ZN4HPHP15f_msg_get_queueExx

(return value) => rax
_rv => rdi
key => rsi
perms => rdx
*/

TypedValue* fh_msg_get_queue(TypedValue* _rv, long long key, long long perms) asm("_ZN4HPHP15f_msg_get_queueExx");

/*
bool HPHP::f_msg_queue_exists(long long)
_ZN4HPHP18f_msg_queue_existsEx

(return value) => rax
key => rdi
*/

bool fh_msg_queue_exists(long long key) asm("_ZN4HPHP18f_msg_queue_existsEx");

/*
bool HPHP::f_msg_send(HPHP::Object const&, long long, HPHP::Variant const&, bool, bool, HPHP::VRefParamValue const&)
_ZN4HPHP10f_msg_sendERKNS_6ObjectExRKNS_7VariantEbbRKNS_14VRefParamValueE

(return value) => rax
queue => rdi
msgtype => rsi
message => rdx
serialize => rcx
blocking => r8
errorcode => r9
*/

bool fh_msg_send(Value* queue, long long msgtype, TypedValue* message, bool serialize, bool blocking, TypedValue* errorcode) asm("_ZN4HPHP10f_msg_sendERKNS_6ObjectExRKNS_7VariantEbbRKNS_14VRefParamValueE");

/*
bool HPHP::f_msg_receive(HPHP::Object const&, long long, HPHP::VRefParamValue const&, long long, HPHP::VRefParamValue const&, bool, long long, HPHP::VRefParamValue const&)
_ZN4HPHP13f_msg_receiveERKNS_6ObjectExRKNS_14VRefParamValueExS5_bxS5_

(return value) => rax
queue => rdi
desiredmsgtype => rsi
msgtype => rdx
maxsize => rcx
message => r8
unserialize => r9
flags => st0
errorcode => st8
*/

bool fh_msg_receive(Value* queue, long long desiredmsgtype, TypedValue* msgtype, long long maxsize, TypedValue* message, bool unserialize, long long flags, TypedValue* errorcode) asm("_ZN4HPHP13f_msg_receiveERKNS_6ObjectExRKNS_14VRefParamValueExS5_bxS5_");

/*
bool HPHP::f_msg_remove_queue(HPHP::Object const&)
_ZN4HPHP18f_msg_remove_queueERKNS_6ObjectE

(return value) => rax
queue => rdi
*/

bool fh_msg_remove_queue(Value* queue) asm("_ZN4HPHP18f_msg_remove_queueERKNS_6ObjectE");

/*
bool HPHP::f_msg_set_queue(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP15f_msg_set_queueERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
queue => rdi
data => rsi
*/

bool fh_msg_set_queue(Value* queue, Value* data) asm("_ZN4HPHP15f_msg_set_queueERKNS_6ObjectERKNS_5ArrayE");

/*
HPHP::Array HPHP::f_msg_stat_queue(HPHP::Object const&)
_ZN4HPHP16f_msg_stat_queueERKNS_6ObjectE

(return value) => rax
_rv => rdi
queue => rsi
*/

Value* fh_msg_stat_queue(Value* _rv, Value* queue) asm("_ZN4HPHP16f_msg_stat_queueERKNS_6ObjectE");

/*
bool HPHP::f_sem_acquire(HPHP::Object const&)
_ZN4HPHP13f_sem_acquireERKNS_6ObjectE

(return value) => rax
sem_identifier => rdi
*/

bool fh_sem_acquire(Value* sem_identifier) asm("_ZN4HPHP13f_sem_acquireERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_sem_get(long long, long long, long long, bool)
_ZN4HPHP9f_sem_getExxxb

(return value) => rax
_rv => rdi
key => rsi
max_acquire => rdx
perm => rcx
auto_release => r8
*/

TypedValue* fh_sem_get(TypedValue* _rv, long long key, long long max_acquire, long long perm, bool auto_release) asm("_ZN4HPHP9f_sem_getExxxb");

/*
bool HPHP::f_sem_release(HPHP::Object const&)
_ZN4HPHP13f_sem_releaseERKNS_6ObjectE

(return value) => rax
sem_identifier => rdi
*/

bool fh_sem_release(Value* sem_identifier) asm("_ZN4HPHP13f_sem_releaseERKNS_6ObjectE");

/*
bool HPHP::f_sem_remove(HPHP::Object const&)
_ZN4HPHP12f_sem_removeERKNS_6ObjectE

(return value) => rax
sem_identifier => rdi
*/

bool fh_sem_remove(Value* sem_identifier) asm("_ZN4HPHP12f_sem_removeERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_shm_attach(long long, long long, long long)
_ZN4HPHP12f_shm_attachExxx

(return value) => rax
_rv => rdi
shm_key => rsi
shm_size => rdx
shm_flag => rcx
*/

TypedValue* fh_shm_attach(TypedValue* _rv, long long shm_key, long long shm_size, long long shm_flag) asm("_ZN4HPHP12f_shm_attachExxx");

/*
bool HPHP::f_shm_detach(long long)
_ZN4HPHP12f_shm_detachEx

(return value) => rax
shm_identifier => rdi
*/

bool fh_shm_detach(long long shm_identifier) asm("_ZN4HPHP12f_shm_detachEx");

/*
bool HPHP::f_shm_remove(long long)
_ZN4HPHP12f_shm_removeEx

(return value) => rax
shm_identifier => rdi
*/

bool fh_shm_remove(long long shm_identifier) asm("_ZN4HPHP12f_shm_removeEx");

/*
HPHP::Variant HPHP::f_shm_get_var(long long, long long)
_ZN4HPHP13f_shm_get_varExx

(return value) => rax
_rv => rdi
shm_identifier => rsi
variable_key => rdx
*/

TypedValue* fh_shm_get_var(TypedValue* _rv, long long shm_identifier, long long variable_key) asm("_ZN4HPHP13f_shm_get_varExx");

/*
bool HPHP::f_shm_has_var(long long, long long)
_ZN4HPHP13f_shm_has_varExx

(return value) => rax
shm_identifier => rdi
variable_key => rsi
*/

bool fh_shm_has_var(long long shm_identifier, long long variable_key) asm("_ZN4HPHP13f_shm_has_varExx");

/*
bool HPHP::f_shm_put_var(long long, long long, HPHP::Variant const&)
_ZN4HPHP13f_shm_put_varExxRKNS_7VariantE

(return value) => rax
shm_identifier => rdi
variable_key => rsi
variable => rdx
*/

bool fh_shm_put_var(long long shm_identifier, long long variable_key, TypedValue* variable) asm("_ZN4HPHP13f_shm_put_varExxRKNS_7VariantE");

/*
bool HPHP::f_shm_remove_var(long long, long long)
_ZN4HPHP16f_shm_remove_varExx

(return value) => rax
shm_identifier => rdi
variable_key => rsi
*/

bool fh_shm_remove_var(long long shm_identifier, long long variable_key) asm("_ZN4HPHP16f_shm_remove_varExx");


} // !HPHP

