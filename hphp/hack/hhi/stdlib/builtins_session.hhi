<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int PHP_SESSION_DISABLED = 0;
const int PHP_SESSION_NONE = 1;
const int PHP_SESSION_ACTIVE = 2;

<<__PHPStdLib>>
function session_set_cookie_params($lifetime, $path = null, $domain = null, $secure = null, $httponly = null) { }
<<__PHPStdLib>>
function session_get_cookie_params() { }
<<__PHPStdLib>>
function session_name($newname = null) { }
<<__PHPStdLib>>
function session_module_name($newname = null) { }
<<__PHPStdLib>>
function session_set_save_handler($open, $close, $read, $write, $destroy, $gc) { }
<<__PHPStdLib>>
function session_save_path($newname = null) { }
<<__PHPStdLib>>
function session_id($newid = null) { }
<<__PHPStdLib>>
function session_regenerate_id(bool $delete_old_session = false) { }
<<__PHPStdLib>>
function session_cache_limiter($new_cache_limiter = null) { }
<<__PHPStdLib>>
function session_cache_expire($new_cache_expire = null) { }
<<__PHPStdLib>>
function session_encode() { }
<<__PHPStdLib>>
function session_decode(string $data) { }
<<__PHPStdLib>>
function session_start() { }
<<__PHPStdLib>>
function session_destroy() { }
<<__PHPStdLib>>
function session_unset() { }
<<__PHPStdLib>>
function session_commit() { }
<<__PHPStdLib>>
function session_write_close() { }
<<__PHPStdLib>>
function session_register($var_names, ...) { }
<<__PHPStdLib>>
function session_register_shutdown() { }
<<__PHPStdLib>>
function session_is_registered($varname) { }
<<__PHPStdLib>>
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
