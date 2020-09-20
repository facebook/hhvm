<?hh
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream
 * Source code: ext/standard/formatted_print.c
 * Alias to functions:
 */

function writeAndDump($fp, $format, $args)
{
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
$file = __SystemLib\hphp_test_tmppath('vfprintf_basic.phpt.txt');
$fp = fopen( $file, "a+" );

// Test vfprintf()
writeAndDump( $fp, "Foo is %d and %s", varray[ 30, 'bar' ] );
writeAndDump( $fp, "%s %s %s", varray[ 'bar', 'bar', 'bar' ] );
writeAndDump( $fp, "%d digit", varray[ '54' ] );
writeAndDump( $fp, "%b %b", varray[ true, false ] );
writeAndDump( $fp, "%c %c %c", varray[ 65, 66, 67 ] );
writeAndDump( $fp, "%e %E %e", varray[ 1000, 2e4, +2e2 ] );
writeAndDump( $fp, "%02d", varray[ 50 ] );
writeAndDump( $fp, "Testing %b %d %f %s %x %X", varray[ 9, 6, 2.5502, "foobar", 15, 65 ] );

// Close handle
fclose( $fp );

echo "===DONE===\n";

unlink( $file );
}
