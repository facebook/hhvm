<?hh <<__EntryPoint>> function main(): void {
$a = darray[1 => 2];
shuffle(inout $a);
var_dump($a);

$a = darray[1 => 2];
array_multisort1(inout $a);
var_dump($a);
}
