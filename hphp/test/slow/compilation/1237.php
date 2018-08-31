<?php

class A {
 public static $foo = 123;
}
 function foo() {
 return 'foo';
}

 <<__EntryPoint>>
function main_1237() {
$a = foo();
 var_dump(A::$$a);
}
