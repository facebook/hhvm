<?php

class A {
 static protected $foo = 11;
   function foo() {
 var_dump(A::$foo);
}
}
 class B extends A {
 static public $foo;
}
 var_dump(B::$foo);
 B::$foo = 123;
 A::foo();
