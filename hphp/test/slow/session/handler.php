<?php

/**
 * Avoid actually writing anything to disk,
 * Just make sure the calling semantics work
 */
class MyHandler extends SessionHandler {
  public function open($save_path, $session_name) {
    echo "Open\n";
    return parent::open($save_path, $session_name);
  }

  public function write($session_id, $data) {
    echo "Write\n";
    var_dump($data);
    return true;
  }
}

ini_set("session.save_handler", "files");
$handler = new MyHandler;
session_set_save_handler($handler, false);
session_start();
$_SESSION['foo'] = 'bar';
session_write_close();
