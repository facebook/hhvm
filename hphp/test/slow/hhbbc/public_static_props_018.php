<?php

class Something {}

class Asd {
  static $SOMETHING = 'Something';
}

function foo($y) {
  if ($y instanceof Asd::$SOMETHING) {
    echo "was an instance\n";
  } else {
    echo "nope\n";
  }
}

foo(new Something);
foo(2);
