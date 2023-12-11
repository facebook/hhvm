<?hh
/*
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
$array=vec[0,1,2];
try { var_dump (array_splice(inout $array, 1,1,3,4,5,6,7,8,9)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump ($array);
echo "Done\n";
}
