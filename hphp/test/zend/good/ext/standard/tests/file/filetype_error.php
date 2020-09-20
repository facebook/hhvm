<?hh
/*
 * Prototype: string filetype ( string $filename );
 * Description: Returns the type of the file. Possible values are fifo, char,
                dir, block, link, file, and unknown.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***";
/* non-existing file or dir */
print( filetype("/no/such/file/dir") );

/* unknown type */
print( filetype("string") );
print( filetype('100') );

/* No.of args less than expected */
try { print( filetype() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of args greater than expected */
try { print( filetype("file", "file") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n*** Done ***\n";
}
