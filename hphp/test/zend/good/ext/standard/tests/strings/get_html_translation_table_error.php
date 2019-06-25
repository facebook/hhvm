<?hh
/* Prototype  : array get_html_translation_table ( [int $table [, int $quote_style [, string charset_hint]]] )
 * Description: Returns the internal translation table used by htmlspecialchars and htmlentities
 * Source code: ext/standard/html.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing get_html_translation_table() : error conditions ***\n";

// More than expected number of arguments
echo "\n-- Testing get_html_translation_table() function with more than expected no. of arguments --\n";
$table = HTML_ENTITIES;
$quote_style = ENT_COMPAT;
$extra_arg = 10;

try { var_dump( get_html_translation_table($table, $quote_style, "UTF-8", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
