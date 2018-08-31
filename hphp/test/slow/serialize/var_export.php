<?php

class A {
  protected $b = 'b';
  private $c = 'c';
  public $d = 'd';
  public $e = array(array('e'));
}

<<__EntryPoint>>
function main_var_export() {
var_export(new A);
}
