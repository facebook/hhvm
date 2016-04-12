<?php

class GenClass {
  function __construct() { echo "Making GenClass\n"; }
  function __destruct()  { echo "Destroying GenClass\n"; }
  function genInner()    { throw new Exception; yield 5; }
}

function genOuter() {
  $x = (new GenClass)->genInner(); // $x is now a generator containing the only reference to a GenClass
  try {
    yield from $x;
  } catch (Exception $ex) {
    echo "Caught Exception (outer)\n";
  }
  echo "Finished genOuter()\n";
}

$o = genOuter();
try {
  $o->current();
} catch (Exception $ex) {
  echo "Caught Exception (main)\n";
}
