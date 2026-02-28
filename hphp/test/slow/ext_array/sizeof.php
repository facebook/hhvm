<?hh


<<__EntryPoint>>
function main_sizeof() :mixed{
$a = dict[];
$a[0] = 1;
$a[1] = 3;
$a[2] = 5;
var_dump(sizeof($a));

$b = dict[];
$b[0] = 7;
$b[5] = 9;
$b[10] = 11;
var_dump(sizeof($b));

var_dump(sizeof(null));
var_dump(sizeof(false));

$food = dict[
  "fruits" => vec["orange", "banana", "apple"],
  "veggie" => vec["carrot", "collard", "pea"]
];
var_dump(sizeof($food));
}
