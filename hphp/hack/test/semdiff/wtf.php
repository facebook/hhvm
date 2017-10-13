<?php
// Copyright 2004-present Facebook. All Rights Reserved.
// this is the source code from which wtf.1.hhas was
// generated. Checked in here just for future reference
// so we can remember what was going on.
// See T22613433 for more motivation
function f(&$x) {
  $x = 3;
}

class F {
 function m(&$y) {
   f($y);
 }
}

function &h(&$z) {
  return $z;
}

function mymain() {
  $f = new F();
  $a = 1;
  $b =& $a;
  $f->m(h($b));
  var_dump($a);
  var_dump($b);
}

mymain();
