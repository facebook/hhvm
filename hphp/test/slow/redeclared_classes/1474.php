<?hh

function nop($en,$es) {}

class X {
  function bar() {
    var_dump($this);
  }
}

<<__EntryPoint>>
function test() {
  if (__hhvm_intrinsics\launder_value(1)) {
    include '1474-1.inc';
  } else {
    include '1474-2.inc';
  }

  include '1474-classes.inc';

  set_error_handler(nop<>);

  $x = new X;
  $x->bar();
  $x = new V;
  $x->bar();
}
