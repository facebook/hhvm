<?php

class Dtor {
  public function __destruct() {
    echo "dtor\n";
  }
}

$ar = array(1,3,2);

//////////////////////////////////////////////////////////////////////
// Exception test cases with fault funclets and nested FPI regions of
// various complexity.

function func() {
}
function blar() {
  throw new Exception("Hi");
}

function foo() {
  global $ar;
  foreach ($ar as $y) {
    func(blar($y));
    echo "wat\n";
  }
  try {} catch (Exception $x) { echo "Bad\n"; }
}

function case1() { foo(); }

function foo2() {
  global $ar;
  foreach ($ar as $y) {
    func(12, new Dtor(), mt_rand(), blar($y) ? 1024 : -1);
  }
  try {} catch (Exception $x) { echo "Bad\n"; }
}

function case2() { foo2(); }

function foo3() {
  global $ar;
  foreach ($ar as $y) {
    func(12, new Dtor(), mt_rand(), func(blar($y)));
  }
  try {} catch (Exception $x) { echo "Bad\n"; }
}

function case3() { foo3(); }


function foo4() {
  global $ar;
  foreach ($ar as $y) {
    func(12, new Dtor(), mt_rand(), func(mt_rand(), blar($y)));
  }
  try {} catch (Exception $x) { echo "Bad\n"; }
}

function case4() { foo3(); }

try { case1(); } catch (Exception $x) { echo "Good1\n"; }
try { case2(); } catch (Exception $x) { echo "Good2\n"; }
try { case3(); } catch (Exception $x) { echo "Good3\n"; }
try { case4(); } catch (Exception $x) { echo "Good4\n"; }
echo "Done\n";