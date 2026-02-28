<?hh <<__EntryPoint>> function main(): void {
$obj = new stdClass;
$obj->prop = vec[1,2];
list($a,$b) = $obj->prop;
var_dump($a,$b);
}
