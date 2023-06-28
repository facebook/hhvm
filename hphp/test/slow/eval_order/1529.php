<?hh

function test($x) :mixed{
  $a = darray[$a => $x[$a = 'foo']];
  return $a;
}

<<__EntryPoint>>
function main_1529() :mixed{
var_dump(test(darray['foo' => 5]));
}
