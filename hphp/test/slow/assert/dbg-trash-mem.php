<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  private $prop = 0;

  function go() {
    $this->prop = 12;
    $this->prop = 24;
    return $this->prop;
  }
}

var_dump((new C())->go());
