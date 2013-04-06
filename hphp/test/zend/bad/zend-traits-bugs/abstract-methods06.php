<?php
error_reporting(E_ALL);

trait THelloB {
  public function hello() {
    echo 'Hello';
  }  
}

trait THelloA {
  public abstract function hello($a);
}

class TraitsTest1 {
	use THelloA;
	use THelloB;
}



?>