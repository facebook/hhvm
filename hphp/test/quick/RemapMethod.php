<?php

// Regression test for Peephole optimizer's logic for remapping method
// entry points

class Foo {
  protected $message = '';
  protected $code = 0;
  protected $previous = null;
  protected $file;
  protected $line;
  protected $trace;
  final function __init__() {
    while (!empty($this->trace)) {
    }
  }
}

$x = new Foo();
var_dump($x);

