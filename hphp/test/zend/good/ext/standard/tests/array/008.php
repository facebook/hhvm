<?hh
//-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=- TEST 1 -=-=-=-=-
<<__EntryPoint>> function main(): void {
$a = dict[
  0 => 1,
  "big" => 2,
  1 => 2,
  2 => 6,
  3 => 3,
  4 => 5,
  5 => 3,
  6 => 3,
  7 => 454,
  8 => 'some_string',
  9 => 3,
  10 => 3,
  11 => 3,
  12 => 3,
  13 => 3,
  14 => 3,
  15 => 3,
  16 => 3,
  17 => 17
];
$b = vec[2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,17,25,'some_string',7,8,9,109,78,17];
$c = vec[-1,2,1,15,25,17];
echo str_repeat("-=",10)." TEST 1 ".str_repeat("-=",20)."\n";
echo '$a='.var_export($a,TRUE).";\n";
echo '$b='.var_export($b,TRUE).";\n";
echo '$c='.var_export($c,TRUE).";\n";

echo 'array_intersect($a,$b,$c);'."\n";
var_dump(array_intersect($a,$b,$c));

echo 'array_intersect_assoc($a,$b,$c);'."\n";
var_dump(array_intersect_assoc($a,$b,$c));

echo 'array_intersect($a,$b);'."\n";
var_dump(array_intersect($a,$b));

echo 'array_intersect_assoc($a,$b);'."\n";
var_dump(array_intersect_assoc($a,$b));

//-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=- TEST 2 -=-=-=-=-=-
$a = dict[
'a'=>2,
'b'=>'some',
'c'=>'done',
'z'=>'foo',
'f'=>5,
'fan'=>'fen',
'bad'=>'bed',
'gate'=>'web',
7=>18,
9=>25,
11=>42,
12=>42,
45=>42,
73=>'foo',
95=>'some',
'som3'=>'some',
'want'=>'wanna'];


$b = dict[
'a'=>7,
7=>18,
9=>13,
11=>42,
45=>46,
'som3'=>'some',
'foo'=>'some',
'goo'=>'foo',
'f'=>5,
'z'=>'equal',
'gate'=>'web'
];
$c = dict[
'gate'=>'web',
73=>'foo',
95=>'some'
];

echo str_repeat("-=",10)." TEST 2 ".str_repeat("-=",20)."\n";
echo '$a='.var_export($a,TRUE).";\n";
echo '$b='.var_export($b,TRUE).";\n";
echo '$c='.var_export($c,TRUE).";\n";
echo "\n\nResults:\n\n";

echo 'array_intersect($a,$b,$c);'."\n";
var_dump(array_intersect($a,$b,$c));

echo 'array_intersect_assoc($a,$b,$c);'."\n";
var_dump(array_intersect_assoc($a,$b,$c));

echo 'array_intersect($a,$b);'."\n";
var_dump(array_intersect($a,$b));

echo 'array_intersect_assoc($a,$b);'."\n";
var_dump(array_intersect_assoc($a,$b));
}
