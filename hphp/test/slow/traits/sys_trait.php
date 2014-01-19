<?php
class X {
  use LazyIterable;
}
function test() {
  $x = new X;
  var_dump($x->lazy());
}
test();

