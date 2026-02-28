<?hh

class X {
  function foo($p) :mixed{ return $p; }
}
function test($a) :mixed{
  $x = new X;

  while (true) {
    $x->foo(new Exception);
  }
}

// This test was to ensure that new Exception ends up
// with a catch trace, because it *can* throw, eg
// on timeout. Without the catch trace, this test case
// would randomly crash (but with a fairly high probability)
// depending on the exact timing of the timeout.

<<__EntryPoint>>
function main_timeout_init() :mixed{
set_time_limit(1);
;

test(1);
}
