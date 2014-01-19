<?php

error_reporting(E_ALL & ~E_NOTICE);
function foo($a) {
 print $a;
}
 class A {
 function __construct() {
}
}
 $obj = new A(foo(10));
