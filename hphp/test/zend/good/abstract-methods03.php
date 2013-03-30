<?php
error_reporting(E_ALL);

trait THello {
  public abstract function hello();
}

class TraitsTest {
  use THello;
  public function hello() {
    echo 'Hello';
  }
}

$test = new TraitsTest();
$test->hello();
?>