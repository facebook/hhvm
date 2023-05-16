<?hh
/* Prototype: string file_get_contents( string $filename{, bool $use_include_path[,
 *                                      resource $context[, int $offset[, int $maxlen]]]] )
 * Description: Reads entire file into a string
 */

/* Prototype: int file_put_contents( string $filename, mixed $data[, int $flags[, resource $context]] )
 * Description: Write a string to a file
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";



echo "\n-- Testing with  Non-existing file --\n";
print( file_get_contents("/no/such/file/or/dir") );

echo "\n-- Testing No.of arguments less than expected --\n";
try { print( file_get_contents() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { print( file_put_contents() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { print( file_put_contents("junk") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$file_handle = fopen(sys_get_temp_dir().'/'.'file_put_contents.tmp', "w");
echo "\n-- Testing No.of arguments greater than expected --\n";
try { print( file_put_contents("abc.tmp", 12345, 1, $file_handle, "extra_argument") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { print( file_get_contents("abc.tmp", false, $file_handle, 1, 2, "extra_argument") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing for invalid negative maxlen values --";
file_put_contents(sys_get_temp_dir().'/'.'file_put_contents1.tmp', "Garbage data in the file");
var_dump( file_get_contents(sys_get_temp_dir().'/'.'file_put_contents1.tmp', FALSE, NULL, 0, -5)) ;

fclose($file_handle);

echo "\n*** Done ***\n";

unlink(sys_get_temp_dir().'/'.'file_put_contents.tmp');
unlink(sys_get_temp_dir().'/'.'file_put_contents1.tmp');
}
