<?hh
<<__EntryPoint>> function main(): void {
// This fails.
$array = darray[1 => 2];
$a = "1";
unset($array[$a]);
print_r($array);

// Those works.
$array = darray[1 => 2];
$a = 1;
unset($array[$a]);
print_r($array);

$array = darray[1 => 2];
unset($array[1]);
print_r($array);

$array = darray[1 => 2];
$a = 1;
unset($array["1"]);
print_r($array);
}
