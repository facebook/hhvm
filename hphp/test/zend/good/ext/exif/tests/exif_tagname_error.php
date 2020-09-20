<?hh

/* Prototype  :string exif_tagname ( string $index  )
 * Description: Get the header name for an index
 * Source code: ext/exif/exif.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing exif_tagname() : error conditions ***\n";

echo "\n-- Testing exif_tagname() function with no arguments --\n";
try { var_dump( exif_tagname() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing exif_tagname() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( exif_tagname(0x10E, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===Done===";
}
