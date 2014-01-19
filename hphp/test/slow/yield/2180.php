<?php

trait T {
  abstract protected function gpc();
  public function gen() {
    yield $this->gpc();
  }
}
class C1 {
  use T;
  protected function gpc() {
    return 1;
  }
}
class C2 {
  use T;
  protected function gpc() {
    return 2;
  }
}
$obj1 = new C1();
$obj2 = new C2();
$c1 = $obj1->gen();
$c2 = $obj2->gen();
$c1->next();
var_dump($c1->current());
$c2->next();
var_dump($c2->current());
