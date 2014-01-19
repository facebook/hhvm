<?php
error_reporting(-1);

class A {
  public $x;
}

var_dump(empty(new A()));
