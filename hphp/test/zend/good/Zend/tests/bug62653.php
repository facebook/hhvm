<?hh <<__EntryPoint>> function main(): void {
$array = darray["5"=>"bar"];
$foo = "10.0000"; // gettype($foo) = "string"
$foo /= 2; //Makes $foo = 5 but still gettype($foo) = "double"
unset($array[$foo]);
print_r($array);

$array = darray["5"=>"bar"];
$foo = "5";
unset($array[(float)$foo]);
print_r($array);

$array = darray["5"=>"bar"];
$foo = "10.0000";
$foo /= 2; //Makes $foo = 5 but still gettype($foo) = "double"
unset($array[$foo]);
print_r($array);
}
