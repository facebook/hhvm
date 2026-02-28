<?hh
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream
 * Source code: ext/standard/formatted_print.c
 * Alias to functions:
 */

// Open handle
<<__EntryPoint>> function main(): void {
$file = sys_get_temp_dir().'/'.'vfprintf_error3.phpt.txt';
$fp = fopen( $file, "a+" );

echo "\n-- Testing vfprintf() function with wrong variable types as argument --\n";

rewind( $fp );
var_dump( stream_get_contents( $fp ) );
ftruncate( $fp, 0 );
rewind( $fp );

var_dump( vfprintf( $fp, "Foo %y fake", "not available" ) );

rewind( $fp );
var_dump( stream_get_contents( $fp ) );
ftruncate( $fp, 0 );
rewind( $fp );

// Close handle
fclose( $fp );

echo "===DONE===\n";

unlink( $file );
}
