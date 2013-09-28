<?php

trait T {
  static function foo() {
    echo "I'm in class " . get_class() . "\n";
  }
}
class C {
 use T;
 }
class D extends C {
}
trait T2 {
 use T;
 }
trait T3 {
 use T2;
 }
$x = new D();
$x->foo();
C::foo();
D::foo();
T::foo();
T2::foo();
T3::foo();
?>
