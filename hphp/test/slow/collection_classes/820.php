<?php

$m1 = Map {
}
;
$m2 = Map {
}
;
var_dump($m1 == $m2);
$m1['a'] = "123";
$m1['b'] = 73;
$m2['a'] = 123;
$m2['b'] = "73";
var_dump($m1 == $m2);
$m1['c'] = 0;
var_dump($m1 == $m2);
echo "------------------------\n";
$m1 = Map {
'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4}
;
$m1->remove('a');
$m1->remove('c');
$m2 = Map {
'b' => 2, 'd' => 4}
;
var_dump($m1 == $m2);
$m1->remove('d');
var_dump($m1 == $m2);
$m2->remove('d');
var_dump($m1 == $m2);
$m1['d'] = 4;
var_dump($m1 == $m2);
$m2['d'] = 4;
var_dump($m1 == $m2);
echo "------------------------\n";
$m = Map {
}
;
var_dump($m == null);
var_dump($m == false);
var_dump($m == true);
var_dump($m == 1);
var_dump($m == "Map");
echo "------------------------\n";
$m = Map {
'x' => 7}
;
var_dump($m == null);
var_dump($m == false);
var_dump($m == true);
var_dump($m == 1);
var_dump($m == "Map");
