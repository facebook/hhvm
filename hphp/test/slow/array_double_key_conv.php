<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a, $k) {
  echo "============== $k ==================\n";

  $a[$k] = darray[];
  $a[$k][$k] = 'abc';
  var_dump($a);
  var_dump($a[$k][$k]);

  $a[$k][$k] .= 'def';
  var_dump($a);
  var_dump($a[$k][$k]);

  $a[$k][$k][5] = 'g';
  var_dump($a);
  var_dump($a[$k][$k]);

  var_dump($a[$k][$k] ?? false);
  var_dump(idx($a, $k));

  unset($a[$k][$k]);
  var_dump($a);
  unset($a[$k]);
  var_dump($a);
}


<<__EntryPoint>>
function main_array_double_key_conv() {
main(darray[], 1.0E+100);
main(darray[], -1.0E+100);
main(darray[], INF);
main(darray[], -INF);
}
