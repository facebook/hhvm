<?php

class A {
  public function b() {
    $cl = function() {
      return $this->c();
    };
    yield $cl();
  }
  private function c() {
    return 'A';
  }
}

$a = new A;
foreach ($a->b() as $c) {
  print "$c\n";
}
