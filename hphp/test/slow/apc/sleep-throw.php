<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class SleepThrow {
  public $msg;

  function __construct($msg) {
    $this->msg = $msg;
  }
  function __sleep() :mixed{
    throw new Exception($this->msg);
  }
}

function store($k, $a) :mixed{
  try {
    apc_store($k, $a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_sleep_throw() :mixed{
store("a1", vec[new SleepThrow("Sleep throw 1")]);
store("a2", dict["key" => new SleepThrow("Sleep throw 2")]);
}
