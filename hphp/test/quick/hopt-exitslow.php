<?php

class C {
  function foo() {
    if ($this) {
      return true;
    } else {
      return false;
    }
  }
}

var_dump(C::foo());
