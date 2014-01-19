<?php

class A {
 function foo($a = 123) {
 var_dump(__CLASS__);
}
}
 class B extends A {
 function foo($b = 123) {
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
