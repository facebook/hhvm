<?php

class A {
 const X=1;
 function foo($a = self::X) {
 var_dump($a);
}
}
 class B extends A {
 const X=2;
 function foo($b = self::X) {
 var_dump($b);
}
}
 function bar() {
   $obj = new A;
 $obj->foo();
  $obj = new B;
 $obj->foo();
}
 bar();
