<?hh
/* Prototype: array glob ( string $pattern [, int $flags] );
   Description: Find pathnames matching a pattern
*/
<<__EntryPoint>> function main(): void {
$file_path = dirname(__FILE__);

// temp dir created
mkdir("$file_path/glob_error");
// temp file created
$fp = fopen("$file_path/glob_error/wonder12345", "w");
fclose($fp);

echo "*** Testing glob() : error conditions ***\n";

echo "-- Testing glob() with unexpected no. of arguments --\n";
try { var_dump( glob() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( glob(dirname(__FILE__)."/glob_error/wonder12345", GLOB_ERR, 3) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected

echo "\n-- Testing glob() with invalid arguments --\n";
try { var_dump( glob(dirname(__FILE__)."/glob_error/wonder12345", '') ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( glob(dirname(__FILE__)."/glob_error/wonder12345", "string") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
error_reporting(0);
// temp file deleted
unlink(dirname(__FILE__)."/glob_error/wonder12345");
// temp dir deleted
rmdir(dirname(__FILE__)."/glob_error");
}
