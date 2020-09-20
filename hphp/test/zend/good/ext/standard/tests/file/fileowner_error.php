<?hh
/* Prototype: int fileowner ( string $filename )
 * Description: Returns the user ID of the owner of the file, or
 *              FALSE in case of an error.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing fileowner(): error conditions ***\n";
/* Non-existing file or dir */
var_dump( fileowner("/no/such/file/dir") );

/* Invalid arguments */
var_dump( fileowner("string") );
var_dump( fileowner('100') );

/* Invalid no.of arguments */
try { var_dump( fileowner() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( fileowner("/no/such/file", "root") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected

echo "\n*** Done ***\n";
}
