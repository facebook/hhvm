<?hh

function test($i, $j) {
  if ($i & 128) $j++;
  return $j;
}

function main($x) {
  for ($i = 0; $i < 10; $i++) {
    test($x, $x);
  }
  return test($x, $x);
}


<<__EntryPoint>>
function main_testb() {
var_dump(main(242));
}
