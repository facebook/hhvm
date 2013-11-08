<?php

class two {
  public static function foo() {
    echo "two\n";
    static::heh();
  }
}

class three extends two {
  public static function heh() {}
}

class doer {
  public function junk($x) {
    $x->foo();
  }
}

function main() {
  $d = new two;
  $x = new doer;
  $x->junk($d);
}

main();
