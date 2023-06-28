<?hh

function bar2_1() :mixed{ return false; }
function bar2_2() :mixed{ return bar2_1(); }
function bar2_3() :mixed{ return bar2_2(); }

function bar1() :mixed{ return __hhvm_intrinsics\launder_value(true); }
function bar2() :mixed{ return bar2_3(); }
function bar3() :mixed{ return __hhvm_intrinsics\launder_value(true); }

function foo() :mixed{
  if (bar1()) return "abc";
  if (bar2()) return "def";
  if (bar3()) return 123;
  return null;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(foo());
}
