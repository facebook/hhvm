<?hh <<__EntryPoint>> function main(): void {
$a = array(1 => 2);
shuffle(&$a);
var_dump($a);

$a = array(1 => 2);
array_multisort1(inout $a);
var_dump($a);
}
