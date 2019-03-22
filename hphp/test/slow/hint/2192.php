<?php
function decide() {

  return mt_rand(0, 0);
}


<<__EntryPoint>>
function main_2192() {

if (decide()) {
  class X {
    public function generator() {
      yield 0;
      yield 1;
    }
  }
}
 else {
  class X {
    public function generator() {
      yield 1;
      yield 2;
    }
  }
}
$x = new X;
foreach ($x->generator() as $v) {
  var_dump($v);
}
}
