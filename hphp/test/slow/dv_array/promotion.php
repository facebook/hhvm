<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function dump($x) {
  var_dump($x);
  var_dump(is_varray($x));
  var_dump(is_darray($x));
}

function test() {
  $x = varray['a', 'b', 'c'];
  dump($x);

  echo "====================================\n";
  $x[] = 'd';
  dump($x);

  echo "====================================\n";
  $x[0] = 'x';
  dump($x);

  echo "====================================\n";
  $x[4] = 'z';
  dump($x);

  echo "====================================\n";
  $x['k'] = 'j';
  dump($x);

  echo "====================================\n";
  $x = varray['a', 'b', 'c'];
  $x[100] = 'x';
  dump($x);
}

<<__EntryPoint>>
function main_promotion() {
test();
}
