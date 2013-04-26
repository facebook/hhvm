<?php

class X {
  function foo() {
    return function() {
      return $this->bar();
    }
;
  }
  function bar() {
}
}
