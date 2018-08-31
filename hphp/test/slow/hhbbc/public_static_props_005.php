<?php

class Foo {
  static $heh = array();
  function go() {
    self::$heh = array(self::$heh);
  }
}


<<__EntryPoint>>
function main_public_static_props_005() {
(new Foo)->go();
var_dump(Foo::$heh);
}
