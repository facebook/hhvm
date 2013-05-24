<?php

class Foo {
  function method() {
    $other = $this;
  }
}

$object = new Foo;
$object->prop = "Hello\n";

$object->method();
