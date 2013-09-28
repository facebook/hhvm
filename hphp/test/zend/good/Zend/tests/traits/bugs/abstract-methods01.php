<?php
error_reporting(E_ALL);

trait THello {
  public abstract function hello();
}

class TraitsTest {
	use THello;  
}

$test = new TraitsTest();
$test->hello();
?>