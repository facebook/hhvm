<?hh
<<__EntryPoint>> function main(): void {
list($a, list($b)) = varray[new stdclass, varray[new stdclass]];
var_dump($a, $b);
}
