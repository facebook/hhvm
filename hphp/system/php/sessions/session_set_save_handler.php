<?php

class _SessionForwardingHandler implements SessionHandlerInterface {
  private $open;
  private $close;
  private $read;
  private $write;
  private $destory;
  private $gc;

  public function __construct($open, $close, $read, $write, $destroy, $gc)  {
    try {
      $this->open = $this->validate($open, 1);
      $this->close = $this->validate($close, 2);
      $this->read = $this->validate($read, 3);
      $this->write = $this->validate($write, 4);
      $this->destroy = $this->validate($destroy, 5);
      $this->gc = $this->validate($gc, 6);
    } catch (Exception $e) {
      trigger_error($e->getMessage(), E_USER_WARNING);
      return false;
    }
  }

  public function open($save_path, $session_id) {
    if ($this->open) {
      return call_user_func($this->open, $save_path, $session_id);
    }
  }
  public function close() {
    if ($this->close) {
      return call_user_func($this->close);
    }
  }
  public function read($session_id) {
    if ($this->read) {
      return call_user_func($this->read, $session_id);
    }
  }
  public function write($session_id, $session_data) {
    if ($this->write) {
      return call_user_func($this->write, $session_id, $session_data);
    }
  }
  public function destroy($session_id) {
    if ($this->destroy) {
      return call_user_func($this->destroy, $session_id);
    }
  }
  public function gc($maxlifetime) {
    if ($this->gc) {
      return call_user_func($this->gc, $maxlifetime);
    }
  }
  private function validate($func, $num) {
    if (!is_callable($func)) {
      throw new Exception("Argument $num is not a valid callback");
    }
    return $func;
  }
}

function session_set_save_handler(
    $open,
    $close = null, $read = null, $write = null, $destroy = null, $gc = null) {
  if ($open instanceof SessionHandlerInterface) {
    return hphp_session_set_save_handler($open, $close);
  }
  return hphp_session_set_save_handler(
    new _SessionForwardingHandler($open, $close, $read, $write, $destroy, $gc),
    false
  );
}
