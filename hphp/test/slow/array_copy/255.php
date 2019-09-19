<?hh

function h6() {
  $x = array(1,2,3,4);
  end(inout $x);
  next(inout $x);
  $y = $x;
  array_pop(&$y);
  var_dump(current($x));
  var_dump(current($y));
}

<<__EntryPoint>>
function main_255() {
h6();
}
