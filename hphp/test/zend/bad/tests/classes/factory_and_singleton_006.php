<?php
class test {

  private function __destruct() {
  }
}

$obj = new test;
$obj = NULL;

echo "Done\n";
?>