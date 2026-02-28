<?hh
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream
 * Source code: ext/standard/formatted_print.c
 * Alias to functions:
 */

// Open handle
<<__EntryPoint>> function main(): void {
$file = sys_get_temp_dir().'/'.'vfprintf_error2.phpt.txt';
$fp = fopen( $file, "a+" );
echo "\n-- Testing vfprintf() function with less than expected no. of arguments --\n";
$format = 'string_val';
try { var_dump( vfprintf($fp, $format) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( vfprintf( $fp ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( vfprintf() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Close handle
fclose($fp);

echo "===DONE===\n";

unlink( $file );
}
