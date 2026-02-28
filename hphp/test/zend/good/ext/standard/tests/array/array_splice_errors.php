<?hh
/*
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "\n*** Testing error conditions of array_splice() ***\n";

$int=1;
$array=vec[1,2];
try { var_dump (array_splice()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump (array_splice(inout $int, )); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump (array_splice(inout $array, )); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump (array_splice(inout $int, $int));
$obj= new stdClass;
var_dump (array_splice(inout $obj, 0,1));
echo "Done\n";
}
