<?hh
// Static arrays.
<<__EntryPoint>> function main(): void {
$a = vec[];
var_dump($a);

$a = vec[null];
var_dump($a);

$a = vec[true];
var_dump($a);

$a = vec[42];
var_dump($a);

$a = vec[12.34];
var_dump($a);

$a = vec["hello"];
var_dump($a);

$a = vec[vec[]];
var_dump($a);

$a = vec[null, true, 42, 12.34, "hello", vec[1, vec[2, vec[3]]]];
var_dump($a);
$a = vec[null, true, 42, 12.34, "hello", vec[1, vec[2, vec[3]]]];
var_dump($a);

$a = dict[0 => "0"];
var_dump($a);

$a = dict[42 => "42"];
var_dump($a);

$a = dict["hello" => "world"];
var_dump($a);

// Non-static arrays.
$v = null;
$a = vec[$v];
var_dump($a);

$k = 0;
$a = dict[$k => "0"];
var_dump($a);

$v = "0";
$a = dict[0 => $v];
var_dump($a);

$k = "hello";
$a = dict[$k => "world"];
var_dump($a);

$v = "world";
$a = dict["hello" => $v];
var_dump($a);

$v = 0;
$a = vec[vec[$v]];
var_dump($a);

$v = 0;
$a = vec[vec[$v], vec[0]];
var_dump($a);

$v = 0;
$a = vec[vec[0], vec[$v]];
var_dump($a);
}
