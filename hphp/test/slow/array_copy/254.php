<?hh

function h5() {
  $x = array(1,2,3,4);
  end(inout $x);
  next(inout $x);
  $y = $x;
  $y[] = 4;
  var_dump(current($x));
  var_dump(current($y));
}

<<__EntryPoint>>
function main_254() {
h5();
}
