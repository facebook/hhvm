<?hh

function test(inout $x, inout $y) :mixed{
  $x = false;
  $y = '';
  $y .= 'hello';
  echo $x;
}

<<__EntryPoint>>
function main_1105() :mixed{
  $x = null;
  test(inout $x, inout $x);
}
