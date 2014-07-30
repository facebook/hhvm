<?php

class X {
  function __call($x, $y) {
    echo "ok\n";
  }
}
class Z {
  function x(&$y) {}
}

function main() {
  $z = 2;
  (new X)->x($z);
}
main();
