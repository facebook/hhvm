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

<<__EntryPoint>>
function main_1719() {
test();
}
