<?php

class Foo {
  function bar() {
    $a = function () {
 var_dump(__CLASS__, __FUNCTION__);
}
;
    $a();
  }
}
Foo::bar();
