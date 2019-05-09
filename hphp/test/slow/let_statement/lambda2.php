<?hh // experimental
<<__EntryPoint>> function main(): void {
let f = ($x) ==> $x + 1;
let twice = ($f) ==> (($x) ==> $f($f($x)));
var_dump(twice(f)(1));
}
