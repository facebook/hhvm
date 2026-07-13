<?hh

function test($x) :mixed{
  $a = 'foo';
  $v = $x[$a];
  $k = $a;
  $a = dict[$k => $v];
  return $a;
}

<<__EntryPoint>>
function main_1529() :mixed{
var_dump(test(dict['foo' => 5]));
}
