<?hh

function h1() {
  $x = varray[1,2,3,4];
  next(inout $x);
  $y = $x;
  unset($y[2]);
  var_dump(current($x));
  var_dump(current($y));
}

<<__EntryPoint>>
function main_250() {
h1();
}
