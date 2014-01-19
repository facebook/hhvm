<?php

class one {
  public function foo() {
    echo "one\n";
  }
}

class two {
  public static function foo() {
    echo "two\n";
    if (get_called_class() === "three") {
      static::heh();
    }
  }
}

class three extends two {
  public static function heh() { echo "three\n"; }
}

class doer {
  public function junk($x) {
    $x->foo();
  }
}

function main() {
  $b = new one;
  $d = new two;
  $x = new doer;
  $x->junk($b);
  $x->junk($d);
  $x->junk($b);
  $x->junk($d);
  $x->junk(new three);
  $x->junk($b);
  $x->junk(new three);
  $x->junk($d);
}

main();
