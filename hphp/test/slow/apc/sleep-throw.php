<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class SleepThrow {
  public $msg;

  public function __construct($msg): void {
    $this->msg = $msg;
  }
  public function __sleep(): void {
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
