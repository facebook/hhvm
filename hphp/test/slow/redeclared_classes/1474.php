<?hh

function nop($en,$es) {}

class X {
  function bar() {
    var_dump($this);
  }
}
if (__hhvm_intrinsics\launder_value(1)) {
  include '1474-1.inc';
} else {
  include '1474-2.inc';
}

class V extends U {
}

<<__EntryPoint>>
function test() {
  set_error_handler(fun('nop'));

  $x = new X;
  $x->bar();
  $x = new V;
  $x->bar();
}
