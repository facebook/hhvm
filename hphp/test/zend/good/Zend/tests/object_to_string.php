<?php

class Foo {

  public function __toString() {
    return "Foo";
  }

}

var_dump(textdomain(new Foo()));
$foo = new Foo();
var_dump(textdomain($foo));
var_dump($foo);
