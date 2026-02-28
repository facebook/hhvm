<?hh

function a() :mixed{ return 42.0; }
function b() :mixed{ return 12.0; }
function foo() :mixed{ return dict[0 => a(), 1 => b()]; }
function junk() :mixed{ return mt_rand() ? 1234 : -1; }
function bar() :mixed{
  $x = foo();
  $x[junk()] = 123.0;
  var_dump($x[0]);
  var_dump($x[1]);
}

<<__EntryPoint>>
function main_array_038() :mixed{
bar();
}
