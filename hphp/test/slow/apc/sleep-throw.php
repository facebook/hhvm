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

store("a1", [new SleepThrow("Sleep throw 1")]);
store("a2", ["key" => new SleepThrow("Sleep throw 2")]);
