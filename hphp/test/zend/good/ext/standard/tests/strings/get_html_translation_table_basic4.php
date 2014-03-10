<?php
/* Prototype  : array get_html_translation_table ( [int $table [, int $quote_style [, string charset_hint]]] )
 * Description: Returns the internal translation table used by htmlspecialchars and htmlentities
 * Source code: ext/standard/html.c
*/


echo "*** Testing get_html_translation_table() : basic functionality/Windows-1252 ***\n";

echo "-- with table = HTML_ENTITIES --\n";
$table = HTML_ENTITIES;
$tt = get_html_translation_table($table, ENT_COMPAT, "WINDOWS-1252");
asort( $tt );
var_dump( $tt );

echo "-- with table = HTML_SPECIALCHARS --\n";
$table = HTML_SPECIALCHARS; 
$tt = get_html_translation_table($table, ENT_COMPAT, "WINDOWS-1252");
asort( $tt );
var_dump( $tt );

echo "Done\n";
?>
