<?hh
<<__EntryPoint>> function main(): void {
list($a, list($b)) = array(new stdclass, array(new stdclass));
var_dump($a, $b);
}
