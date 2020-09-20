<?hh
/*
 * Prototype: int fileinode ( string $filename );
 * Description: Returns the inode number of the file, or FALSE in case of an error.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions of fileinode() ***";

/* Non-existing file or dir */
var_dump( fileinode("/no/such/file/dir") );

/* Invalid arguments */
var_dump( fileinode("string") );
var_dump( fileinode('100') );

/* No.of arguments less than expected */
try { var_dump( fileinode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of arguments greater than expected */
try { var_dump( fileinode(__FILE__, "string") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n*** Done ***";
}
