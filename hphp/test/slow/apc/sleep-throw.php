<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class SleepThrow {
  public $msg;

  function __construct($msg) {
    $this->msg = $msg;
  }
  function __sleep() {
    throw new Exception($this->msg);
  }
}

function store($k, $a) {
  try {
    apc_store($k, $a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_sleep_throw() {
store("a1", varray[new SleepThrow("Sleep throw 1")]);
store("a2", darray["key" => new SleepThrow("Sleep throw 2")]);
}
