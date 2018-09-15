<?php

class Asd {
  private static $foo = array(1,false,3, L::A);

  function go() {
    var_dump(Asd::$foo);
    self::$foo[1][] = 2;
    var_dump(Asd::$foo);
    $y = self::$foo[1];
    var_dump($y);
  }
}


<<__EntryPoint>>
function main_static_props_018() {
if (mt_rand()) {
  class L { const A = 2; }
} else {
  class L { const A = 3; }
}

Asd::go();
}
