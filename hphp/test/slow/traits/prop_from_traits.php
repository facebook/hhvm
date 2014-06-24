<?php

trait T {
  private $a = 1;
  private static $sa = 1;
  public $pa = 1;
  public static $spa = 1;

  public function t() {
    var_dump($this->a);
    var_dump(get_object_vars($this));
  }
}

class A {
  use T;

  private $b = 4;
  public $c = 'hi';
  private static $sb = 4;
  public static $sc = 'hi';
}


$a = new A();
$a->t();
