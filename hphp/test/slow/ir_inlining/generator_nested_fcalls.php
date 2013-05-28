<?php

class SecurityLogger {
  private $x, $y;

  public function setResource($x) {
    $this->x = $x;
    return $this;
  }

  public function setExtra($x) {
    $this->y = $x;
    return $this;
  }

  public function log() {
    echo "logging\n";
  }
}

function SecurityLogger($x, $y) {
  return new SecurityLogger();
}

function unique_function() {
  mt_rand();
  return array('1');
}

function foo() {
  $resource_arr = array('a' => 'b');
  // Inlining something which calls a function inside a generator
  // doesn't work.  This test shouldn't inline right now (if it does
  // it crashes).
  SecurityLogger(12, 12)
    ->setResource((array)unique_function())
    ->log();
  yield 12;
}

function main() {
  foreach (foo() as $k) {
}
}
main();
