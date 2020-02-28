<?hh

function h2() {
  $x = varray[1,2,3,4];
  next(inout $x);
  $y = $x;
  $y[] = 4;
  var_dump(current($x));
  var_dump(current($y));
}

<<__EntryPoint>>
function main_251() {
h2();
}
