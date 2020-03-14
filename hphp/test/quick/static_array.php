<?hh
# Static arrays.
<<__EntryPoint>> function main(): void {
$a = varray[];
var_dump($a);

$a = varray[null];
var_dump($a);

$a = varray[true];
var_dump($a);

$a = varray[42];
var_dump($a);

$a = varray[12.34];
var_dump($a);

$a = varray["hello"];
var_dump($a);

$a = varray[varray[]];
var_dump($a);

$a = varray[null, true, 42, 12.34, "hello", varray[1, varray[2, varray[3]]]];
var_dump($a);
$a = varray[null, true, 42, 12.34, "hello", varray[1, varray[2, varray[3]]]];
var_dump($a);

$a = darray[0 => "0"];
var_dump($a);

$a = darray[42 => "42"];
var_dump($a);

$a = darray["hello" => "world"];
var_dump($a);

# Non-static arrays.
$v = null;
$a = varray[$v];
var_dump($a);

$k = 0;
$a = darray[$k => "0"];
var_dump($a);

$v = "0";
$a = darray[0 => $v];
var_dump($a);

$k = "hello";
$a = darray[$k => "world"];
var_dump($a);

$v = "world";
$a = darray["hello" => $v];
var_dump($a);

$v = 0;
$a = varray[varray[$v]];
var_dump($a);

$v = 0;
$a = varray[varray[$v], varray[0]];
var_dump($a);

$v = 0;
$a = varray[varray[0], varray[$v]];
var_dump($a);
}
