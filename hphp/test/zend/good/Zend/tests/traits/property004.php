<?php
error_reporting(E_ALL);

trait THello1 {
  public $hello = "foo";
}

trait THello2 {
  private $hello = "bar";
}

echo "PRE-CLASS-GUARD\n";

class TraitsTest {
	use THello1;
	use THello2;
	public function getHello() {
	    return $this->hello;
	}
}

$t = new TraitsTest;
?>