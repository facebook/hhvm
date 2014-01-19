<?php

class A {
}
 class B extends A {
 static function foo() {
 return new parent();
}
 }
 var_dump(B::foo());
