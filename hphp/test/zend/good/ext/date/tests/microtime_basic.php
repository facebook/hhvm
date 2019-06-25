<?hh
/* 
 * proto mixed microtime([bool get_as_float])
 * Function is implemented in ext/standard/microtime.c
*/ 
<<__EntryPoint>> function main(): void {
var_dump(microtime());
var_dump(microtime(true));
var_dump(microtime(false));

echo "===DONE===\n";
}
