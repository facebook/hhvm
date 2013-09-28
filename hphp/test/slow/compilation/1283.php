<?php

class A {
 function f($a) {
}
 }
$obj = new A;
$obj->f(date('m/d/y H:i:s', 123456789));
$v = date("m",123456789)+1;
