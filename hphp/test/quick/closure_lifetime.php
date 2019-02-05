<?php

class blah {
  private string $t = "";
  public function __construct() { $this->t = "hi\n"; }
  public function foo() { return function() { return $this->t; }; }
}

function main() {
  $k = (new blah)->foo(); // only reference to obj is in the closure
  echo $k();
  unset($k);
  echo "done\n";
}

main();
