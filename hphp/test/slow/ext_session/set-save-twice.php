<?php

class ProxySessionHandler implements SessionHandlerInterface {

  public function __construct() {
    $this->handler = new SessionHandler;
  }

  public function open($a, $b) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return $this->handler->open($a, $b);
  }

  public function close() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return $this->handler->close();
  }

  public function destroy($key) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return $this->handler->destroy($key);
  }

  public function gc($maxlifetime) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return $this->handler->gc($maxlifetime);
  }

  public function read($id) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return $this->handler->read($id);
  }

  public function write($id, $data) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return $this->handler->write($id, $data);
  }

}

$handler = new ProxySessionHandler();
session_set_save_handler($handler, false);
session_set_save_handler($handler, false);
session_start();

$_SESSION['a'] = 'a';
var_dump($_SESSION['a']);
