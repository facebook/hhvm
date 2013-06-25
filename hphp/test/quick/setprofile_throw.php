<?php


function throwing_profiler($case, $func) {
  if ($case == 'enter' && ($func == 'bar' || $func == 'baz')) {
    throw new Exception("yeah");
  }
}

function bar() { echo "bar()\n"; }
function baz() { echo "baz()\n"; }

function foo($f) {
  $f();
}

function signal_thrower() {
  echo "signal throwing\n";
  throw new Exception("Sig exception");
}

function func_to_enter() {}

class DtorObj {
  public function __construct($x) { $this->x = $x; echo "__ctor $x\n"; }
  public function __destruct() { echo "__dtor " . $this->x . "\n"; }
}

// During function exit
function func_entry() {
  $x1 = new DtorObj(1);
  $x2 = new DtorObj(2);
  $x3 = new DtorObj(3);
  $x4 = new DtorObj(4);
  $x5 = new DtorObj(5);

  $x6 = new DtorObj(6);
  $x7 = new DtorObj(7);
  $x8 = new DtorObj(8);
  $x9 = new DtorObj(9);
  $xa = new DtorObj(0xa);

  $xb = new DtorObj(0xb);
  $xc = new DtorObj(0xc);
  $xd = new DtorObj(0xd);
  $xe = new DtorObj(0xe);
  $xf = new DtorObj(0xf);

  posix_kill(posix_getpid(), 10);

  // Surprise flags are checked on function entry.
  func_to_enter();
}

// During backward branches
function func_backward() {
  $x1 = new DtorObj(1);
  $x2 = new DtorObj(2);
  $x3 = new DtorObj(3);
  $x4 = new DtorObj(4);
  $x5 = new DtorObj(5);

  $x6 = new DtorObj(6);
  $x7 = new DtorObj(7);
  $x8 = new DtorObj(8);
  $x9 = new DtorObj(9);
  $xa = new DtorObj(0xa);

  $xb = new DtorObj(0xb);
  $xc = new DtorObj(0xc);
  $xd = new DtorObj(0xd);
  $xe = new DtorObj(0xe);
  $xf = new DtorObj(0xf);

  posix_kill(posix_getpid(), 10);

  // Surprise flags are checked on backward branch.
  for ($i = 0; $i < 2; ++$i) {}
}

function main() {
// Test throwing on function entry
  fb_setprofile('throwing_profiler');
  try { foo('bar'); } catch (Exception $x) { echo "Caught\n"; }
  try { foo('baz'); } catch (Exception $x) { echo "Caught\n"; }
  fb_setprofile(null);

// Test throwing exceptions from surprise flags things (OOM, signals,
// and req timeout all work this way).

  pcntl_signal(10, 'signal_thrower');

  try { func_entry(); } catch (Exception $x) { echo "caught\n"; }
  try { func_backward(); } catch (Exception $x) { echo "caught\n"; }
}

main();
