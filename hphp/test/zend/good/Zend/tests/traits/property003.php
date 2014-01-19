<?php
error_reporting(E_ALL);

trait THello1 {
  public $hello;
}

trait THello2 {
  private $hello;
}

echo "PRE-CLASS-GUARD\n";

class TraitsTest {
	use THello1;
	use THello2;
}

echo "POST-CLASS-GUARD\n";

$t = new TraitsTest;
$t->hello = "foo";
?>