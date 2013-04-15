<?php

trait t {
  public static function f() {
    a::priv();
  }
}

class a {
  use t;

  private static function priv() {
    echo "Private in a\n";
  }
}

function main() {
  a::f();
  t::f();
}
main();
