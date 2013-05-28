<?php

class A {
 function a() {
 echo "A
";
 }
}
function test() {
 $obj = new A();
 $obj->a();
 }
test();
