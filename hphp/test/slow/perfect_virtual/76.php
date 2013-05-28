<?php

class A {
 function foo() {
 var_dump(__CLASS__);
}
}
 class B extends A {
 function foo() {
 var_dump(__CLASS__);
}
}
 function bar() {
   $obj = new A;
 $obj->foo();
  $obj = new B;
 $obj->foo();
}
 bar();
