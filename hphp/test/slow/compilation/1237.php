<?php

class A {
 public static $foo = 123;
}
 $a = foo();
 function foo() {
 return 'foo';
}
 var_dump(A::$$a);
