<?hh
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream
 * Source code: ext/standard/formatted_print.c
 * Alias to functions:
 */

function writeAndDump($fp, $format, $args)
:mixed{
    ftruncate( $fp, 0 );
    $length = vfprintf( $fp, $format, $args );
    rewind( $fp );
    $content = stream_get_contents( $fp );
    var_dump( $content );
    var_dump( $length );
}
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : basic functionality ***\n";

// Open handle
$file = sys_get_temp_dir().'/'.'vfprintf_basic.phpt.txt';
$fp = fopen( $file, "a+" );

// Test vfprintf()
writeAndDump( $fp, "Foo is %d and %s", vec[ 30, 'bar' ] );
writeAndDump( $fp, "%s %s %s", vec[ 'bar', 'bar', 'bar' ] );
writeAndDump( $fp, "%d digit", vec[ '54' ] );
writeAndDump( $fp, "%b %b", vec[ true, false ] );
writeAndDump( $fp, "%c %c %c", vec[ 65, 66, 67 ] );
writeAndDump( $fp, "%e %E %e", vec[ 1000, 2e4, +2e2 ] );
writeAndDump( $fp, "%02d", vec[ 50 ] );
writeAndDump( $fp, "Testing %b %d %f %s %x %X", vec[ 9, 6, 2.5502, "foobar", 15, 65 ] );

// Close handle
fclose( $fp );

echo "===DONE===\n";

unlink( $file );
}
