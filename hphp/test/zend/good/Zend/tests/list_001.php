<?hh
<<__EntryPoint>> function main(): void {
list($a, list($b)) = varray[new stdClass, varray[new stdClass]];
var_dump($a, $b);
}
