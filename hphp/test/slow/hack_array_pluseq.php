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
$a1 = varray[1, 2, 3];
$a2 = varray["1", "b", "c"];

$v1 = vec[1, 2, 3];
$d1 = dict['a' => 1, 'b' => 2, 'c' => 3];
$k1 = keyset[1, 2, 3];

main(vec[], $a1, $a2);
main(dict[], $a1, $a2);
main(keyset[], $a1, $a2);

main(vec[], varray[], varray[]);
main(dict[], darray[], darray[]);
main(keyset[], varray[], varray[]);

main($v1, $a1, $a2);
main($d1, $a1, $a2);
main($k1, $a1, $a2);

main($v1, varray[], varray[]);
main($d1, darray[], darray[]);
main($k1, varray[], varray[]);

main($a1, vec[], $v1);
main($a1, dict[], $d1);
main($a1, keyset[], $k1);

main($a1, $v1, vec[]);
main($a1, $d1, dict[]);
main($a1, $k1, keyset[]);

main(varray[], vec[], $v1);
main(darray[], dict[], $d1);
main(varray[], keyset[], $k1);

main(varray[], $v1, vec[]);
main(darray[], $d1, dict[]);
main(varray[], $k1, keyset[]);
}
