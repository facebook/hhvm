<?php

function blah() {
  throw new Exception('asd');
}

class something {
  public function __destruct() {
    echo "~something\n";
    $k = debug_backtrace();#[1]['object'];
    var_dump($k[1]['class'], isset($k[1]['object']));
  }
  public function yo() {
    blah();
  }
}

class Bar {
  public function foo() {
    $k = new something();
    echo "wat\n";
    blah();
    echo "eh\n";
  }
}

function main() {
  (new Bar)->foo();
}

main();
