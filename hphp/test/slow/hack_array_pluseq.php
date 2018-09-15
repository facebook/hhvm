<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($a, $a2, $a3) {
  $a += $a2;
  $a += $a3;
  var_dump($a);
}

function main($a, $a2, $a3) {
  try {
    test($a, $a2, $a3);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}


<<__EntryPoint>>
function main_hack_array_pluseq() {
$a1 = array(1, 2, 3);
$a2 = array("1", "b", "c");

$v1 = vec[1, 2, 3];
$d1 = dict['a' => 1, 'b' => 2, 'c' => 3];
$k1 = keyset[1, 2, 3];

main(vec[], $a1, $a2);
main(dict[], $a1, $a2);
main(keyset[], $a1, $a2);

main(vec[], [], []);
main(dict[], [], []);
main(keyset[], [], []);

main($v1, $a1, $a2);
main($d1, $a1, $a2);
main($k1, $a1, $a2);

main($v1, [], []);
main($d1, [], []);
main($k1, [], []);

main($a1, vec[], $v1);
main($a1, dict[], $d1);
main($a1, keyset[], $k1);

main($a1, $v1, vec[]);
main($a1, $d1, dict[]);
main($a1, $k1, keyset[]);

main([], vec[], $v1);
main([], dict[], $d1);
main([], keyset[], $k1);

main([], $v1, vec[]);
main([], $d1, dict[]);
main([], $k1, keyset[]);
}
