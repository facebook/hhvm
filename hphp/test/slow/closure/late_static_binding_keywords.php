<?php

class Foo {
  public function bar() {
    $x = function() {
      var_dump(static::class);
      var_dump(self::class);
    };
    $x();
  }
}

class Herp extends Foo {
}

(new Herp())->bar();
