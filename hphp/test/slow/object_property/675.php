<?php

class A {
 static private $foo = 11;
 }
 class B extends A {
}
 class C extends B {
 static public $foo;
}
 var_dump(C::$foo);

