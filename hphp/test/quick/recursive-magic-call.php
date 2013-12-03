<?php

class Blah {
  public function __call($x, $y) {
    global $z;
    $z->hoho();
  }
}

$z = new Blah();
function main() {
  global $z;
  $z->whatever();
}

main();;
