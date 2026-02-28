<?hh

function test($x) :mixed{
  $y = ($x ? 5 : 3) + 5;
  return vec[$y + 1, $y + 1,];
}


<<__EntryPoint>>
function main_cse_assert() :mixed{
var_dump(test(0));
}
