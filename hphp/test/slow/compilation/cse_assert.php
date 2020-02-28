<?hh

function test($x) {
  $y = ($x ? 5 : 3) + 5;
  return varray[$y + 1, $y + 1,];
}


<<__EntryPoint>>
function main_cse_assert() {
var_dump(test(0));
}
