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
function ftok(string $pathname, string $proj);
<<__PHPStdLib>>
function msg_get_queue(int $key, int $perms = 0666);
<<__PHPStdLib>>
function msg_queue_exists(int $key);
<<__PHPStdLib>>
function msg_send(resource $queue, int $msgtype, $message, bool $serialize,
                  bool $blocking, inout $errorcode);
<<__PHPStdLib>>
function msg_receive(resource $queue, int $desiredmsgtype, inout $msgtype,
                     int $maxsize, inout $message, bool $unserialize,
                     int $flags, inout $errorcode);
<<__PHPStdLib>>
function msg_remove_queue(resource $queue);
<<__PHPStdLib>>
function msg_set_queue(resource $queue, $data);
<<__PHPStdLib>>
function msg_stat_queue(resource $queue);
<<__PHPStdLib>>
function sem_acquire(resource $sem_identifier, bool $nowait = false);
<<__PHPStdLib>>
function sem_get(int $key, int $max_acquire = 1, int $perm = 0666, bool $auto_release = true);
<<__PHPStdLib>>
function sem_release(resource $sem_identifier);
<<__PHPStdLib>>
function sem_remove(resource $sem_identifier);
<<__PHPStdLib>>
function shm_attach(int $shm_key, int $shm_size = 10000, int $shm_flag = 0666);
<<__PHPStdLib>>
function shm_detach(int $shm_identifier);
<<__PHPStdLib>>
function shm_remove(int $shm_identifier);
<<__PHPStdLib>>
function shm_get_var(int $shm_identifier, int $variable_key);
<<__PHPStdLib>>
function shm_has_var(int $shm_identifier, int $variable_key);
<<__PHPStdLib>>
function shm_put_var(int $shm_identifier, int $variable_key, $variable);
<<__PHPStdLib>>
function shm_remove_var(int $shm_identifier, int $variable_key);
