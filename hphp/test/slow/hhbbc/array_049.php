<?hh

function a() { return 1; }
function foo() {
  $x = varray[a()];
  $x[] = 0;
  $x[1] += 12;
  return $x;
}
function d() {
  $y = foo();
  var_dump($y);
}

<<__EntryPoint>>
function main_array_049() {
d();
}
