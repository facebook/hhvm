<?hh

function nop($en,$es) :mixed{}

class X {
  function bar() :mixed{
    var_dump($this);
  }
}

<<__EntryPoint>>
function test() :mixed{
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
