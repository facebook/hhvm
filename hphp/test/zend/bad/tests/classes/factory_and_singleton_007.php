<?php
class test {

  protected function __clone() {
  }
}

$obj = new test;
$clone = clone $obj;
$obj = NULL;

echo "Done\n";
?>