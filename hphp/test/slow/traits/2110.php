<?php

trait T {
  private function foo() {
}
}
class C {
  protected function foo() {
}
}
class D extends C {
  use T;
  protected function foo() {
}
}
class E extends C {
  public static function test($obj) {
    $obj->foo();
  }
}
$d = new D;
E::test($d);
echo "Done\n";
