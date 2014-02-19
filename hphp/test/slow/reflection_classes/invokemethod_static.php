<?php

class Foo {
  public static function bar() {
    var_dump(func_get_args());
  }
}

class Bar {
}

function main() {
  $m = new ReflectionMethod('Foo', 'bar');
  $m->invoke($m);
  $m->invoke(new Bar());
}

main();
