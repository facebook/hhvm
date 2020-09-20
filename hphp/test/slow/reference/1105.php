<?hh

function test(inout $x, inout $y) {
  $x = false;
  $y .= 'hello';
  echo $x;
}

<<__EntryPoint>>
function main_1105() {
  $x = null;
  test(inout $x, inout $x);
}
