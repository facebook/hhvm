<?hh
/* Prototype  : string quoted_printable_decode  ( string $str  )
 * Description: Convert a quoted-printable string to an 8 bit string
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing quoted_printable_decode() : error conditions ***\n";

echo "\n-- Testing quoted_printable_decode() function with no arguments --\n";
try { var_dump( quoted_printable_decode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing quoted_printable_decode() function with more than expected no. of arguments --\n";
$str = b"=FAwow-factor=C1=d0=D5=DD=C5=CE=CE=D9=C5=0A=
=20=D4=cf=D2=C7=CF=D7=D9=C5=
=20=
=D0=
=D2=CF=C5=CB=D4=D9";
$extra_arg = 10;
try { var_dump( quoted_printable_decode($str, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
