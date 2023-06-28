<?hh


<<__EntryPoint>>
function main_545() :mixed{
$a1 = varray[];
$a2 = varray[null];
$a3 = varray[true];
$a4 = varray[false];
$a5 = varray[0];
$a6 = varray[1];
$a7 = varray[1.0];
$a8 = varray['1.0'];
$a9 = varray[1.23456789e+34];
$a13 = varray[1.7976931348623157e+308];
$a10 = varray[1E666];
$a11 = varray[1E666/1E666];
$a12 = varray["a bc"];
$a13 = varray["\xc1 bc"];
$a14 = varray[null, true, false, 0, 1, 1.0,             '1.0', 1.23456789e+34,             1.7976931348623157e+308, 1E666,             1E666/1E666, "a bc",             "\xc1 bc"];
$a15 = darray[
  "" => true,
  1 => 1.0,
  '1.0' => 1.23456789e+34,
  (int)1.7976931348623157e+308 => 1E666,
  (int)(1E666/1E666) => "a bc",
  2 => "\xc1 bc"
];
$a16 = darray[
  "" => true,
  1 => 1.0,
  '1.0' => 1.23456789e+34,
  (int)1.7976931348623157e+308 => 1E666,
  (int)(1E666/1E666) => "a bc",
  2 => "\xc1 bc",
  3 => darray[
    "" => true,
    0 => varray[],
    1 => 1.0,
    '1.0' => 1.23456789e+34,
    (int)1.7976931348623157e+308 => 1E666,
    (int)(1E666/1E666) => "a bc",
    2 => "\xc1 bc"
  ]
];
var_dump($a1);
var_dump($a2);
var_dump($a3);
var_dump($a4);
var_dump($a5);
var_dump($a6);
var_dump($a7);
var_dump($a8);
var_dump($a9);
var_dump($a10);
var_dump($a11);
var_dump($a12);
var_dump($a13);
var_dump($a14);
var_dump($a15);
var_dump($a16);
}
