<?php
error_reporting(E_ALL);

trait THello1 {
  private function hello() {
    echo 'Hello';
  }
}

trait THello2 {
  private function hello() {
    echo 'Hello';
  }
}

class TraitsTest {
	use THello1;
	use THello2;
}
?>