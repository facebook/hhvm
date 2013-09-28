<?php
abstract class A {
  abstract function foo();
}
interface B {
  function bar();
}
class C extends A implements B {
}
?>