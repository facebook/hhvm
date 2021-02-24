<?hh

function bar2_1() { return false; }
function bar2_2() { return bar2_1(); }
function bar2_3() { return bar2_2(); }

function bar1() { return __hhvm_intrinsics\launder_value(true); }
function bar2() { return bar2_3(); }
function bar3() { return __hhvm_intrinsics\launder_value(true); }

function foo() {
  if (bar1()) return "abc";
  if (bar2()) return "def";
  if (bar3()) return 123;
  return null;
}

<<__EntryPoint>>
function main() {
  var_dump(foo());
}
