<?hh <<__EntryPoint>> function main(): void {
$obj = new stdClass;
list($a,$b) = $obj->prop = [1,2];
var_dump($a,$b);
}
