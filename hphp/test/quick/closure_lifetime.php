<?php

class blah {
  public function __destruct() { echo "~blah()\n"; }
  public function foo() { return function() { return "hi"; }; }
}

function main() {
  $k = (new blah)->foo(); // only reference to obj is in the closure
  $k();
  unset($k);
  echo "done\n";
}

main();
