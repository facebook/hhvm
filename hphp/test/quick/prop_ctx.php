<?php

trait t {
  public static function f($o) {
    var_dump($o->prop);
  }
  public static function set($o, $v) {
    $o->prop = $v;
    var_dump($o);
  }
}

class a {
  use t;
  private $prop = 'I am private in a';
}

class b extends a {
  public $prop = 'I am public in b';
}

function main() {
  $b = new b();
  $b->f($b);
  t::f($b);

  $b->set($b, 'new value');
  t::set($b, 'newer value');

  $a = new a();
  $a->f($a);
  t::f($a);
}
main();
