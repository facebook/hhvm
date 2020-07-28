<?hh

function h3() {
  $x = varray[1,2,3,4];
  end(inout $x);
  $y = $x;
  array_pop(inout $y);
  var_dump(current($x));
  var_dump(current($y));
}

<<__EntryPoint>>
function main_252() {
h3();
}
