<?hh

/* Prototype: int fseek ( resource $handle, int $offset [, int $whence] );
   Description: Seeks on a file pointer

   Prototype: bool rewind ( resource $handle );
   Description: Rewind the position of a file pointer

   Prototype: int ftell ( resource $handle );
   Description: Tells file pointer read/write position
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing ftell() : error conditions ***\n";
// zero argument
echo "-- Testing ftell() with zero argument --\n";
try { var_dump( ftell() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of args
echo "-- Testing ftell() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
try { var_dump( ftell($fp, 10) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// ftell on a file handle which is already closed
echo "-- Testing ftell with closed/unset file handle --";
fclose($fp);
var_dump(ftell($fp));

// ftell on a file handle which is unset
$file_handle = fopen(__FILE__, "r");
unset($file_handle); //unset file handle
try { var_dump( ftell($file_handle) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
