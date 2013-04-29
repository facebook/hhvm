<?php
error_reporting(E_ALL);

interface MyInterface {
	public function a();
}

trait THello implements MyInterface {
  public function a() {
    echo 'A';
  }
}

?>