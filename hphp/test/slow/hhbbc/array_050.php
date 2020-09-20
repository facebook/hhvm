<?hh

function a() { return 1; }
function foo() {
  $x = varray[a()];
  $x[] = 0;
  $x[1]++;
  return $x;
}
function d() {
  $y = foo();
  var_dump($y);
}

<<__EntryPoint>>
function main_array_050() {
d();
}
