<?hh
/* Prototype: bool is_executable ( string $filename );
   Description: Tells whether the filename is executable
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing is_executable(): error conditions ***\n";
try { var_dump( is_executable() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected no of arguments

try { var_dump( is_executable(1, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected no. of arguments

echo "\n*** Testing is_exceutable() on non-existent directory ***\n";
var_dump( is_executable(dirname(__FILE__)."/is_executable") );

echo "Done\n";
}
