<?hh <<__EntryPoint>> function main(): void {
$obj = new stdClass;
list($a,$b) = $obj->prop = varray[1,2];
var_dump($a,$b);
}
