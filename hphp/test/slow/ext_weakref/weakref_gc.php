<?php
// Source php weakref extension
class Foo {
  private $ptr = null;
  public function set($p) {
    $this->ptr = $p;
  }
}
function foo() {
  $a = new Foo();
  $b = new Foo();
  $a->set($b);
  $b->set($a);
  return new WeakRef($a);
}
class Leaker {
  private $ptr = null;
  public function set($p) {
    $this->ptr = $p;
  }
};
function leaker($acquire) {
  $a = new Leaker();
  $b = new Leaker();
  $a->set($b);
  $b->set($a);
  $wr = new WeakRef($a);
  if ($acquire) {
    $wr->acquire();
  }
  return $wr;
}
function factorial($n) {
  if ($n <= 1) return 1;
  return $n * factorial($n-1);
}
// Test if unreachable.
$wr = leaker(false);

// Test if only reachable throuh WR.
$wr2 = leaker(true);

// Test that we don't release if object is marked.
$a = new StdClass();
$wr3 = new WeakRef($a);

// Test x2 WRs on same object.
$wr4 = leaker(true);
$wr5 = new WeakRef($wr4->get());
$wr4->release();

// Check that a weak-ref to a weak-ref works.
$wr6 = leaker(true);
$wr7 = new WeakRef($wr6);
$wr6->release();

// We need to cleanup tvBuiltinReturn, we do this by creating a garbage object,
// and just passing it around. (Otherwise, it's possible one of the Leakers
// above could be in tvBuiltinReturn, and marked as reachable.)
$_ = foo();  // clear tvReturn.
// Similarly, we need to make sure the PHP stack doesn't have an object that's
// accidentally reachable if the stack is conservatively scanned.
factorial(10);  // clears the stack.

gc_collect_cycles();
var_dump($wr->valid(), $wr->get());  // NULL
var_dump($wr2->valid(), $wr2->get());  // !NULL
var_dump($wr3->valid(), $wr3->get());  // !NULL
var_dump($wr4->valid(), $wr4->get());  // NULL
var_dump($wr5->valid(), $wr5->get());  // NULL
var_dump($wr6->valid(), $wr6->get());  // NULL
var_dump($wr7->valid(), $wr7->get());  // !NULL
