<?php

class A {
}
 class B extends A {
 static function foo() {
 return new self();
}
 }
 var_dump(B::foo());
