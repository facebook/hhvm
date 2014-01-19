<?php
define("FOO", 123);
trait T {
  private $blah = FOO;
  public function test() {
    var_dump($this->blah);
  }
}
class C {
  use T;
}
$obj = new C;
$obj->test();
