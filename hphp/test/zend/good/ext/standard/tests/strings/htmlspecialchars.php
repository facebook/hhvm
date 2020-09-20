<?hh
/* Prototype: string htmlspecialchars ( string $string [, int $quote_style [, string $charset]] );
   Description: Convert special characters to HTML entities
*/

/* retrieving htmlspecialchars from the ANSI character table */
<<__EntryPoint>> function main(): void { echo "*** Retrieving htmlspecialchars for 256 characters ***\n";
for($i=0; $i<256; $i++)
var_dump( bin2hex( htmlspecialchars(b"chr($i)") ) );

/* giving strange arguments */
echo "\n*** Testing htmlspecialchars() with NULL as first, second and third argument ***\n";
var_dump( htmlspecialchars("<br>", 0, 'iso-8859-1') );
var_dump( htmlspecialchars("<br>", ENT_NOQUOTES, '') );
var_dump( htmlspecialchars("<br>", ENT_QUOTES, '') );
var_dump( htmlspecialchars("<br>", ENT_COMPAT, '') );
var_dump( htmlspecialchars('', 0, '') );

/* giving long string to check for proper memory re-allocation */
echo "\n*** Checking a long string for proper memory allocation ***\n";
var_dump( htmlspecialchars("<br>Testing<p>New file.</p><p><br>File <b><i><u>WORKS!!!</i></u></b></p><br><p>End of file!!!</p>", ENT_QUOTES, 'iso-8859-1' ) );

/* Giving a normal string */
echo "\n*** Testing a normal string with htmlspecialchars() ***\n";
var_dump( htmlspecialchars("<br>Testing<p>New file.</p> ", ENT_QUOTES, 'iso-8859-1' ) );

/* checking behavior of quote */
echo "\n*** Testing htmlspecialchars() on a quote...\n";
$str = "A 'quote' is <b>bold</b>";
var_dump( htmlspecialchars($str) );
var_dump( htmlspecialchars($str, ENT_QUOTES) );
var_dump( htmlspecialchars($str, ENT_NOQUOTES) );
var_dump( htmlspecialchars($str, ENT_COMPAT) );

echo "\n*** Testing error conditions ***\n";
/* zero argument */
try { var_dump( htmlspecialchars() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* giving arguments more than expected */
var_dump( htmlspecialchars("<br>",ENT_QUOTES,'iso-8859-1', true) );

echo "Done\n";
}
