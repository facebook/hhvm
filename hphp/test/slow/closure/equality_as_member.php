<?php

class Foo {
  public function __construct() {
    $this->foo = function() { var_dump('herpderp'); };
  }
}

var_dump((new Foo()) == (new Foo()));
