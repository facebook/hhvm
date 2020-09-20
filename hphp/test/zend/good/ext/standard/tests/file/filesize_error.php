<?hh
/* 
 * Prototype   : int filesize ( string $filename );
 * Description : Returns the size of the file in bytes, or FALSE 
 *               (and generates an error of level E_WARNING) in case of an error.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing filesize(): error conditions ***";

/* Non-existing file or dir */
var_dump( filesize("/no/such/file") );
var_dump( filesize("/no/such/dir") );

/* No.of arguments less than expected */
try { var_dump( filesize() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of arguments greater than expected */
try { var_dump( filesize(__FILE__, 2000) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "\n";

echo "*** Done ***\n";
}
