<?php
error_reporting(E_ALL);

trait THello1 {
  private $foo;
}

trait THello2 {
  private $foo;
}

echo "PRE-CLASS-GUARD-TraitsTest\n";
error_reporting(E_ALL & ~E_STRICT); // ensuring that it is only for E_STRICT

class TraitsTest {
	use THello1;
	use THello2;
}

error_reporting(E_ALL | E_STRICT);

echo "PRE-CLASS-GUARD-TraitsTest2\n";

class TraitsTest2 {
	use THello1;
	use THello2;
}

var_dump(property_exists('TraitsTest', 'foo'));
var_dump(property_exists('TraitsTest2', 'foo'));
?>