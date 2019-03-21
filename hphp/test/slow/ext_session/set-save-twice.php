<?php

class ProxySessionHandler implements SessionHandlerInterface {

  public function __construct() {
    $this->handler = new SessionHandler;
  }

  public function open($a, $b) {
    var_dump(__FUNCTION__);
    var_dump(array($a, $b));
    return $this->handler->open($a, $b);
  }

  public function close() {
    var_dump(__FUNCTION__);
    var_dump(array());
    return $this->handler->close();
  }

  public function destroy($key) {
    var_dump(__FUNCTION__);
    var_dump(array($key));
    return $this->handler->destroy($key);
  }

  public function gc($maxlifetime) {
    return $this->handler->gc($maxlifetime);
  }

  public function read($id) {
    var_dump(__FUNCTION__);
    var_dump(array($id));
    return $this->handler->read($id);
  }

  public function write($id, $data) {
    var_dump(__FUNCTION__);
    var_dump(array($id, $data));
    return $this->handler->write($id, $data);
  }

}


<<__EntryPoint>>
function main_set_save_twice() {
$handler = new ProxySessionHandler();
session_set_save_handler($handler, false);
session_set_save_handler($handler, false);
session_start();

$_SESSION['a'] = 'a';
var_dump($_SESSION['a']);
}
