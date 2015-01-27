<?php
ini_set('track_errors', 1);

class Foo {
  public function __destruct() {
    throw new Exception("FOO");
  }
}

$f = new Foo();
