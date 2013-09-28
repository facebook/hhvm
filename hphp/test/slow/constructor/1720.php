<?php

class A {
  function a() {
 echo "A
";
 }
  function __construct() {
 echo "cons
";
 }
}
 function test() {
 $obj = new A();
 $obj->a();
 }
 test();
