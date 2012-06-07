<?php

class c {
  public $prop = 'old value';
  public function __get($n) {
    return "You asked for property $n";
  }
  public function __set($n, $v) {
    echo "Pretending to set $n to $v\n";
  }
}

function propget($o, $p) {
  return $o->$p;
}

function propset($o, $p, $v) {
  $o->$p = $v;
}

function main() {
  $c = new c();
  var_dump(propget($c, 'prop'));
  propset($c, 'prop', 'new value');
  var_dump(propget($c, 'prop'));

  var_dump(propget($c, 'fakeprop'));
  propset($c, 'fakeprop2', 'blah');
}
main();
