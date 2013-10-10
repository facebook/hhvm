<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class C{
  public function __construct($v) {
    $this->prop = $v;
  }
}

function &blah() {
  $x = &new C(rand());
  return $x;
}

var_dump(blah());
