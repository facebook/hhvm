<?hh <<__EntryPoint>> function main(): void {
var_dump(boolval(false));
var_dump(boolval(NULL));
var_dump(boolval(""));
var_dump(boolval(0));
var_dump(boolval(varray[]));

var_dump(boolval(true));
var_dump(boolval("abc"));
var_dump(boolval(0.5));
var_dump(boolval(100));
var_dump(boolval(new stdClass()));
var_dump(boolval(STDIN));
}
