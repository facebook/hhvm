<?php

class A {
  public function b() {
    $cl = function() {
      yield $this->c();
    };
    yield $cl();
  }
  private function c() {
    return 'A';
  }
}

$a = new A;
foreach ($a->b() as $c) {
  foreach ($c as $d) {
    print "$d\n";
  }
}
