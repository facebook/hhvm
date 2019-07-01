<?hh
/* Prototype: string get_include_path  ( void  )
 * Description: Gets the current include_path configuration option

*/
<<__EntryPoint>> function main(): void {
echo "*** Testing get_include_path()\n";

var_dump(get_include_path());

if (ini_get("include_path") == get_include_path()) {
    echo "PASSED\n";
} else {
    echo "FAILED\n";
}

echo "\nError cases:\n";
try { var_dump(get_include_path(TRUE)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


echo "===DONE===\n";
}
