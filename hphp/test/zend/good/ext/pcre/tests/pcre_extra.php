<?hh
<<__EntryPoint>> function main(): void {
var_dump(preg_match('/\y/', '\y'));
var_dump(preg_match('/\y/X', '\y'));
}
