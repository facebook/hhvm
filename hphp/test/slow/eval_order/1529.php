<?hh

function test($x) :mixed{
  $a = dict[$a => $x[$a = 'foo']];
  return $a;
}

<<__EntryPoint>>
function main_1529() :mixed{
var_dump(test(dict['foo' => 5]));
}
