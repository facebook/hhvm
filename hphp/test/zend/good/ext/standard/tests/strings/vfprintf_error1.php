<?hh
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream
 * Source code: ext/standard/formatted_print.c
 * Alias to functions:
 */

// Open handle
<<__EntryPoint>> function main(): void {
$file = sys_get_temp_dir().'/'.'vfprintf_error1.phpt.txt';
$fp = fopen( $file, "a+" );
echo "\n-- Testing vfprintf() function with more than expected no. of arguments --\n";
$format = 'string_val';
$args = vec[ 1, 2 ];
$extra_arg = 10;
try { var_dump( vfprintf( $fp, $format, $args, $extra_arg ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( vfprintf( $fp, "Foo %d", vec[6], "bar" ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Close handle
fclose($fp);

echo "===DONE===\n";

unlink( $file );
}
