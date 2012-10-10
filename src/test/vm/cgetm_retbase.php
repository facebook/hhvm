<?php
function f() { $a = array(); $a[] = 1; return $a; }
function test() {
  return f()[0];
}
test();
