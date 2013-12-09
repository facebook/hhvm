<?php

class Foo {}

function main(Foo $x = null) {
  if (!$x) {
    echo is_object($x) . "\n";
  } else {
    echo is_object($x) . "\n";
  }
  return 2;
}

main();


