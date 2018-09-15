<?php

class C {
  protected function foo() {
 echo "C::foo\n";
 }
}
trait T {
  protected function foo() {
 echo "T::foo\n";
 }
}
class D extends C {
  use T;
}
class E extends C {
  public static function test($obj) {
    $obj->foo();
  }
}

<<__EntryPoint>>
function main_2111() {
$d = new D;
E::test($d);
echo "Done\n";
}
