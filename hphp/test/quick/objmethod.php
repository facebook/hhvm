<?php

trait t {
  public static function f($o) {
    $o->blah();
  }
}

class c {
  use t;
  private function blah() {
    echo "private function\n";
  }
}

function main() {
  $o = new c;
  c::f($o);
  t::f($o);
}
main();
