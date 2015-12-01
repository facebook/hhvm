<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
const int PHP_SESSION_DISABLED = 0;
const int PHP_SESSION_NONE = 1;
const int PHP_SESSION_ACTIVE = 2;

function session_set_cookie_params($lifetime, $path = null, $domain = null, $secure = null, $httponly = null) { }
function session_get_cookie_params() { }
function session_name($newname = null) { }
function session_module_name($newname = null) { }
function session_set_save_handler($open, $close, $read, $write, $destroy, $gc) { }
function session_save_path($newname = null) { }
function session_id($newid = null) { }
function session_regenerate_id($delete_old_session = false) { }
function session_cache_limiter($new_cache_limiter = null) { }
function session_cache_expire($new_cache_expire = null) { }
function session_encode() { }
function session_decode($data) { }
function session_start() { }
function session_destroy() { }
function session_unset() { }
function session_commit() { }
function session_write_close() { }
function session_register($var_names, ...) { }
function session_register_shutdown() { }
function session_is_registered($varname) { }
function session_status() { }

interface SessionHandlerInterface {

  abstract public function close(): bool;
  abstract public function destroy($session_id): bool;
  abstract public function gc($maxlifetime): bool;
  abstract public function open($save_path , $name): bool;
  abstract public function read($session_id): string;
  abstract public function write($session_id, $session_data): bool;
}

class SessionHandler implements SessionHandlerInterface {

  public function close(): bool;
  public function create_sid (): string;
  public function destroy($session_id): bool;
  public function gc($maxlifetime): bool;
  public function open($save_path , $name): bool;
  public function read($session_id): string;
  public function write($session_id, $session_data): bool;
}
