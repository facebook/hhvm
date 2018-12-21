<?php
function x() { return 'x'; }
function foo() {
  $x = 2;
  $z = x();
  $y =& $$z;
  return $y;
}

<<__EntryPoint>>
function main_vgetn() {
foo();
}
