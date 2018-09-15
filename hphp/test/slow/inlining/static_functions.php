<?php

final class Constants {
  public static function gen1() {
    yield 'foo';
  }

  public static function gen2() {
    yield 'bar';
  }

  public static function gen3($s) {
    yield $s;
  }
}

function main() {
  $g = Constants::gen1();
  var_dump($g->current());
  $g = Constants::gen2();
  var_dump($g->current());
  $g = Constants::gen3("baz");
  var_dump($g->current());
}


<<__EntryPoint>>
function main_static_functions() {
main();
}
