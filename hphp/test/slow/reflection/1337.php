<?php

class A {
 static $a = 10;
 public $b = 20;
}
$obj = new A();
 var_dump(get_object_vars($obj));
