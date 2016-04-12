<?php

assert_options(ASSERT_ACTIVE, 1);
assert_options(ASSERT_WARNING, 1);

class C {
  public function __construct() {
    echo "in construct\n";
  }

  public function __toString() {
    echo "in toString\n";
    return "toString";
  }
}

ini_set("zend.assertions", 1);
$x = assert(false, new C);
var_dump($x);

echo "-\n";

ini_set("zend.assertions", 0);
$x = assert(false, new C);
var_dump($x);

ini_set("zend.assertions", -1);
var_dump(ini_get("zend.assertions"));

$f = 'assert';
$f(false, new C);
