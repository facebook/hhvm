<?php

error_reporting(E_ALL | E_STRICT);
trait T {
  protected function f() {
    return 1;
  }
  private function p() {
    return 2;
  }
  function g() {
    return $this->f();
  }
  function h() {
    return $this->p();
  }
}
class C {
  use T;
}
$c = new C;
echo $c->g();
echo $c->h();
