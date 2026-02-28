<?hh


<<__EntryPoint>>
function main_545() :mixed{
$a1 = vec[];
$a2 = vec[null];
$a3 = vec[true];
$a4 = vec[false];
$a5 = vec[0];
$a6 = vec[1];
$a7 = vec[1.0];
$a8 = vec['1.0'];
$a9 = vec[1.23456789e+34];
$a13 = vec[1.7976931348623157e+308];
$a10 = vec[1E666];
$a11 = vec[1E666/1E666];
$a12 = vec["a bc"];
$a13 = vec["\xc1 bc"];
$a14 = vec[null, true, false, 0, 1, 1.0,             '1.0', 1.23456789e+34,             1.7976931348623157e+308, 1E666,             1E666/1E666, "a bc",             "\xc1 bc"];
$a15 = dict[
  "" => true,
  1 => 1.0,
  '1.0' => 1.23456789e+34,
  (int)1.7976931348623157e+308 => 1E666,
  (int)(1E666/1E666) => "a bc",
  2 => "\xc1 bc"
];
$a16 = dict[
  "" => true,
  1 => 1.0,
  '1.0' => 1.23456789e+34,
  (int)1.7976931348623157e+308 => 1E666,
  (int)(1E666/1E666) => "a bc",
  2 => "\xc1 bc",
  3 => dict[
    "" => true,
    0 => vec[],
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
