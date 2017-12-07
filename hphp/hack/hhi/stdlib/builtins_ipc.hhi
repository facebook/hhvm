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
<<__PHPStdLib>>
function ftok($pathname, $proj) { }
<<__PHPStdLib>>
function msg_get_queue($key, $perms = 0666) { }
<<__PHPStdLib>>
function msg_queue_exists($key) { }
<<__PHPStdLib>>
function msg_send($queue, $msgtype, $message, $serialize = true, $blocking = true, &$errorcode = null) { }
<<__PHPStdLib>>
function msg_receive($queue, $desiredmsgtype, &$msgtype, $maxsize, &$message, $unserialize = true, $flags = 0, &$errorcode = null) { }
<<__PHPStdLib>>
function msg_remove_queue($queue) { }
<<__PHPStdLib>>
function msg_set_queue($queue, $data) { }
<<__PHPStdLib>>
function msg_stat_queue($queue) { }
<<__PHPStdLib>>
function sem_acquire($sem_identifier, bool $nowait = false) { }
<<__PHPStdLib>>
function sem_get($key, $max_acquire = 1, $perm = 0666, $auto_release = true) { }
<<__PHPStdLib>>
function sem_release($sem_identifier) { }
<<__PHPStdLib>>
function sem_remove($sem_identifier) { }
<<__PHPStdLib>>
function shm_attach($shm_key, $shm_size = 10000, $shm_flag = 0666) { }
<<__PHPStdLib>>
function shm_detach($shm_identifier) { }
<<__PHPStdLib>>
function shm_remove($shm_identifier) { }
<<__PHPStdLib>>
function shm_get_var($shm_identifier, $variable_key) { }
<<__PHPStdLib>>
function shm_has_var($shm_identifier, $variable_key) { }
<<__PHPStdLib>>
function shm_put_var($shm_identifier, $variable_key, $variable) { }
<<__PHPStdLib>>
function shm_remove_var($shm_identifier, $variable_key) { }
