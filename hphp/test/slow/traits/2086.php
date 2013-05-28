<?php

trait Too {
  function bar() {
    $a = function () {
 var_dump(__CLASS__, __FUNCTION__);
}
;
    $a();
  }
}
class Foo {
 use Too;
 }
Foo::bar();
