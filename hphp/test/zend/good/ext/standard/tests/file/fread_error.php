<?hh
/*
 Prototype: string fread ( resource $handle [, int $length] );
 Description: reads up to length bytes from the file pointer referenced by handle.
   Reading stops when up to length bytes have been read, EOF (end of file) is
   reached, (for network streams) when a packet becomes available, or (after
   opening userspace stream) when 8192 bytes have been read whichever comes first.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";
$filename = __FILE__;
$file_handle = fopen($filename, "r");

// zero argument
echo "-- Testing fread() with zero argument --\n";
try { var_dump( fread() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of args
echo "-- Testing fread() with more than expected number of arguments --\n";
try { var_dump( fread($file_handle, 10, $file_handle) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// invalid length argument
echo "-- Testing fread() with invalid length arguments --\n";
$len = 0;
var_dump( fread($file_handle, $len) );
$len = -10;
var_dump( fread($file_handle, $len) );

// fwrite() on a file handle which is already closed
echo "-- Testing fwrite() with closed/unset file handle --\n";
fclose($file_handle);
var_dump( fread($file_handle,0) );

// fwrite on a file handle which is unset
$fp = fopen($filename, "r");
unset($fp); //unset file handle
try { var_dump( fread($fp,10) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( fclose($fp) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
