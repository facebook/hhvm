<?hh
<<__EntryPoint>> function main(): void {
$m = null;
var_dump(preg_match_with_matches('/\PN+/', '123abc', inout $m));
var_dump($m);

var_dump(preg_match('/\P{N}+/A', '123abc'));
var_dump(preg_match('/^\P{N}+/', '123abc'));
var_dump(preg_match('/^\P{N}+/A', '123abc'));
}
