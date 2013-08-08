<?php

class X {
  function foo(&$a) {
    return $this;
  }
}

class Y {
  function foo($a) {
    if ($a) {
      $this->foo($a[0])->foo($a[0]);
    }
    return $this;
  }
}
