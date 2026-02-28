<?hh
<<__EntryPoint>> function main(): void {
list($a, list($b)) = vec[new stdClass, vec[new stdClass]];
var_dump($a, $b);
}
