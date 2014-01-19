<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

class canary {
  function __destruct() {
    var_dump(__METHOD__);
  }
}

class FooBar extends Exception {
  public $c;
  function __construct() {
    var_dump(__METHOD__);
  }
  function __destruct() {
    var_dump(__METHOD__);
  }

  protected function initTrace() {
    $this->c = new canary;
    global $save;
    $save = $this;
    throw new Exception;
  }
}

function test() {
  try {
    $x = new FooBar(1,2);
  } catch (Exception $e) {
    var_dump('Caught');
    unset($e);
  }
}

test();
var_dump(isset($save->c));
$save = null;
