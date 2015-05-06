<?php

function go($wakkawakka) {
  $wakkawakka .= new dtor(2);
  mt_rand();
  echo $wakkawakka;
}

class dtor {
  private $i;
  function __construct($i) { $this->i = $i; }
  function __destruct() {
    var_dump(debug_backtrace()[1]['args']);
    echo "dtor: $this->i\n";
  }
  function __toString() { echo "toString: $this->i\n"; return "a"; }
}

function x() {
  $z = "asd".(new dtor(0));
  go($z);
}
x();
