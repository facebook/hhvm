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

long fh_ftok(Value* pathname, Value* proj) asm("_ZN4HPHP6f_ftokERKNS_6StringES2_");

TypedValue* fh_msg_get_queue(TypedValue* _rv, long key, long perms) asm("_ZN4HPHP15f_msg_get_queueEll");

bool fh_msg_queue_exists(long key) asm("_ZN4HPHP18f_msg_queue_existsEl");

bool fh_msg_remove_queue(Value* queue) asm("_ZN4HPHP18f_msg_remove_queueERKNS_6ObjectE");

bool fh_msg_set_queue(Value* queue, Value* data) asm("_ZN4HPHP15f_msg_set_queueERKNS_6ObjectERKNS_5ArrayE");

Value* fh_msg_stat_queue(Value* _rv, Value* queue) asm("_ZN4HPHP16f_msg_stat_queueERKNS_6ObjectE");

bool fh_msg_send(Value* queue, long msgtype, TypedValue* message, bool serialize, bool blocking, TypedValue* errorcode) asm("_ZN4HPHP10f_msg_sendERKNS_6ObjectElRKNS_7VariantEbbRKNS_14VRefParamValueE");

bool fh_msg_receive(Value* queue, long desiredmsgtype, TypedValue* msgtype, long maxsize, TypedValue* message, bool unserialize, long flags, TypedValue* errorcode) asm("_ZN4HPHP13f_msg_receiveERKNS_6ObjectElRKNS_14VRefParamValueElS5_blS5_");

bool fh_sem_acquire(Value* sem_identifier) asm("_ZN4HPHP13f_sem_acquireERKNS_6ObjectE");

bool fh_sem_release(Value* sem_identifier) asm("_ZN4HPHP13f_sem_releaseERKNS_6ObjectE");

TypedValue* fh_sem_get(TypedValue* _rv, long key, long max_acquire, long perm, bool auto_release) asm("_ZN4HPHP9f_sem_getElllb");

bool fh_sem_remove(Value* sem_identifier) asm("_ZN4HPHP12f_sem_removeERKNS_6ObjectE");

TypedValue* fh_shm_attach(TypedValue* _rv, long shm_key, long shm_size, long shm_flag) asm("_ZN4HPHP12f_shm_attachElll");

bool fh_shm_detach(long shm_identifier) asm("_ZN4HPHP12f_shm_detachEl");

bool fh_shm_remove(long shm_identifier) asm("_ZN4HPHP12f_shm_removeEl");

TypedValue* fh_shm_get_var(TypedValue* _rv, long shm_identifier, long variable_key) asm("_ZN4HPHP13f_shm_get_varEll");

bool fh_shm_has_var(long shm_identifier, long variable_key) asm("_ZN4HPHP13f_shm_has_varEll");

bool fh_shm_put_var(long shm_identifier, long variable_key, TypedValue* variable) asm("_ZN4HPHP13f_shm_put_varEllRKNS_7VariantE");

bool fh_shm_remove_var(long shm_identifier, long variable_key) asm("_ZN4HPHP16f_shm_remove_varEll");

} // namespace HPHP
