<?php
/* Prototype  : int vfprintf(resource stream, string format, array args)
 * Description: Output a formatted string into a stream
 * Source code: ext/standard/formatted_print.c
 * Alias to functions:
 */

echo "*** Testing vfprintf() : variation functionality ***\n";

// Open handle
$file = 'vfprintf_variation1.phpt.txt';
$fp = fopen( $file, 'a+' );

$funset = fopen( __FILE__, 'r' );
unset( $funset );

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

// Test vfprintf()
writeAndDump( $fp, "format", null );
writeAndDump( $fp, "Foo is %d and %s", array( 30, 'bar' ) );
writeAndDump( $fp, "Foobar testing", array() );
writeAndDump( $fp, "%s %s %s", array( 'bar', 'bar', 'bar' ) );
writeAndDump( $fp, "%02d", array( 50 ) );
writeAndDump( $fp, "", array() );
writeAndDump( $fp, "Testing %b %d %f %o %s %x %X", array( 9, 6, 2.5502, 24, "foobar", 15, 65 ) );
@writeAndDump( $funset, "Foo with %s", array( 'string' ) );
@writeAndDump( new FooClass(), "Foo with %s", array( 'string' ) );

// Close handle
fclose( $fp );

?>
===DONE===
<?php error_reporting(0); ?>
<?php

$file = 'vfprintf_variation1.phpt.txt';
unlink( $file );

?>
