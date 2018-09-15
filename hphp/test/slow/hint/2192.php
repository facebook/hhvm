<?php
function decide() {
  global $WHICH;
  return $WHICH;
}


<<__EntryPoint>>
function main_2192() {
$WHICH = 0;
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
