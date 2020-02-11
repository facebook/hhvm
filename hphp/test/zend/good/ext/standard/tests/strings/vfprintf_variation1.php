<?hh
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream
 * Source code: ext/standard/formatted_print.c
 * Alias to functions:
 */










class FooClass
{
    public function __toString()
    {
        return "Object";
    }
}

// Output facilitating function
function writeAndDump($fp, $format, $args)
{
    try { ftruncate( $fp, 0 ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $length = vfprintf( $fp, $format, $args );
    try { rewind( $fp ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    try { $content = stream_get_contents( $fp ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    var_dump( $content );
    var_dump( $length );
}
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : variation functionality ***\n";

// Open handle
$file = 'vfprintf_variation1.phpt.txt';
$fp = fopen( $file, 'a+' );

$funset = fopen( __FILE__, 'r' );
unset( $funset );

// Test vfprintf()
writeAndDump( $fp, "format", null );
writeAndDump( $fp, "Foo is %d and %s", varray[ 30, 'bar' ] );
writeAndDump( $fp, "Foobar testing", varray[] );
writeAndDump( $fp, "%s %s %s", varray[ 'bar', 'bar', 'bar' ] );
writeAndDump( $fp, "%02d", varray[ 50 ] );
writeAndDump( $fp, "", varray[] );
writeAndDump( $fp, "Testing %b %d %f %o %s %x %X", varray[ 9, 6, 2.5502, 24, "foobar", 15, 65 ] );
@writeAndDump( $funset, "Foo with %s", varray[ 'string' ] );
@writeAndDump( new FooClass(), "Foo with %s", varray[ 'string' ] );

// Close handle
fclose( $fp );

echo "===DONE===\n";
error_reporting(0);
$file = 'vfprintf_variation1.phpt.txt';
unlink( $file );
}
