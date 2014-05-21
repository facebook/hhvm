<?php

class A {
  protected $b = 'b';
  private $c = 'c';
  public $d = 'd';
  public $e = array(array('e'));
}
var_export(new A);
