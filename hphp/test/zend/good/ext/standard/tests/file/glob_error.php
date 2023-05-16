<?hh
/* Prototype: array glob ( string $pattern [, int $flags] );
   Description: Find pathnames matching a pattern
*/
<<__EntryPoint>> function main(): void {
$dirname = sys_get_temp_dir().'/'.'glob_error';

// temp dir created
mkdir($dirname);
// temp file created
$fp = fopen("$dirname/wonder12345", "w");
fclose($fp);

echo "*** Testing glob() : error conditions ***\n";

echo "-- Testing glob() with unexpected no. of arguments --\n";
try { var_dump( glob() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( glob("$dirname/wonder12345", GLOB_ERR, 3) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected

echo "\n-- Testing glob() with invalid arguments --\n";
try { var_dump( glob("$dirname/wonder12345", '') ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( glob("$dirname/wonder12345", "string") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";

unlink("$dirname/wonder12345");
rmdir($dirname);
}
