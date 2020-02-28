<?hh

function h4() {
  $x = varray[1,2,3,4];
  end(inout $x);
  next(inout $x);
  $y = $x;
  unset($y[2]);
  var_dump(current($x));
  var_dump(current($y));
}

<<__EntryPoint>>
function main_253() {
h4();
}
