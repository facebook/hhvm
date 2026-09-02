<?hh

/* Prototype: int fseek ( resource $handle, int $offset [, int $whence] );
   Description: Seeks on a file pointer

   Prototype: bool rewind ( resource $handle );
   Description: Rewind the position of a file pointer

   Prototype: int ftell ( resource $handle );
   Description: Tells file pointer read/write position
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing rewind() : error conditions ***\n";
// zero argument
echo "-- Testing rewind() with zero argument --\n";
try { var_dump( rewind() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of args
echo "-- Testing rewind() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
try { var_dump( rewind($fp, 10) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// rewind on a file handle which is already closed
echo "-- Testing rewind() with closed file handle --";
fclose($fp);
var_dump(rewind($fp));

echo "Done\n";
}
