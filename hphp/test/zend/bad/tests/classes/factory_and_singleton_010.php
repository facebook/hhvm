<?php
class test {

  private function __destruct() {
  	echo __METHOD__ . "\n";
  }
}

$obj = new test;

?>
===DONE===