<?php

class A {
 static protected $foo = 11;
 }
 class B extends A {
}
 class C extends B {
 static public $foo;
}
 var_dump(C::$foo);

