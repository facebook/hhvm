<?php

/*
 * This is a test that checks that CGetM in the IR properly cleans the
 * stack before operations that can throw.
 */

class Dtor {
  public function __construct() { echo "hi\n"; }
  public function __destruct() { echo "ah\n"; }
}

class Something {
  public $wat;

  public function __construct() {
    $this->wat = new Dtor();
  }
}

class Unsetter {
  private $x;

  public function __construct() {
    unset($this->x);
  }

  public function useX($k) {
    echo "sup\n";
    $z = $k->wat + $this->x;
    return $z;
  }
}

function thrower() {
  throw new Exception("wat");
}

function main() {
  set_error_handler('thrower');
  $k = new Unsetter;
  $k->useX(new Something());
}

try {
  main();
} catch (Exception $x) {
  echo "out\n";
}