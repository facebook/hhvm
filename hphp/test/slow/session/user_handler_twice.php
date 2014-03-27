<?php

class Foo {

  public function open($path, $name) { return true;}
  public function close() { return true;}
  public function read($id) { return ''; }
  public function write($id, $data) { return true; }
  public function destroy($id) { return true;}
  public function gc($maxlifetime) { return true;}

  public function __construct() {
    session_cache_limiter(false);
    var_dump(
    session_set_save_handler(
      array($this, 'open'),
      array($this, 'close'),
      array($this, 'read'),
      array($this, 'write'),
      array($this, 'destroy'),
      array($this, 'gc')
    )
    );
  }
}

ini_set('session.use_cookies', 0);
new Foo();
session_start();
session_status();
print("--- session_set_save_handler ---\n");
new Foo();
print("--- Changing session handler ---\n");
var_dump(ini_set('session.save_handler', 'user'));
