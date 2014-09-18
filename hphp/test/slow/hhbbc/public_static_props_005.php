<?php

class Foo {
  static $heh = array();
  function go() {
    self::$heh = array(self::$heh);
  }
}

(new Foo)->go();
var_dump(Foo::$heh);
