<?hh

function a() { return 42.0; }
function b() { return 12.0; }
function foo() { return array(a(), b()); }
function junk() { return mt_rand() ? 1234 : -1; }
function bar() {
  $x = foo();
  $x[junk()] = 123.0;
  var_dump($x[0]);
  var_dump($x[1]);
}
bar();
