<?php

function regular($a, $b, $c) {
  echo __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

function variadic($a, ...$args) {
  echo __FUNCTION__, "\n";
  var_dump($a, $args);
}

class C {
  public static function stRegular($a, $b, $c) {
    echo __METHOD__, "\n";
    var_dump($a, $b, $c);
  }

  public function regular($a, $b, $c) {
    echo __METHOD__, "\n";
    var_dump($a, $b, $c);
  }

  public static function stVariadic($a, ...$args) {
    echo __METHOD__, "\n";
    var_dump($a, $args);
  }

  public function variadic($a, ...$args) {
    echo __METHOD__, "\n";
    var_dump($a, $args);
  }
}

function main() {
  echo "Done\n"; return; // only a parser test for now

  $args = array('a', 'b', 'c');

  regular(...$args);
  variadic(...$args);
  C::stRegular(...$args);
  C::stVariadic(...$args);
  $inst = new C();
  $inst->regular(...$args);
  $inst->variadic(...$args);
}

main();
