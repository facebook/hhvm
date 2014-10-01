<?php
/* Prototype  : array get_html_translation_table ( [int $table [, int $quote_style [, string charset_hint]]] )
 * Description: Returns the internal translation table used by htmlspecialchars and htmlentities
 * Source code: ext/standard/html.c
*/

/* Test get_html_translation_table() when table is specified as HTML_ENTITIES */


echo "*** Testing get_html_translation_table() : basic functionality ***\n";

echo "-- with table = HTML_ENTITIES --\n";
$table = HTML_ENTITIES;
$tt = get_html_translation_table($table, ENT_COMPAT, "UTF-8");
asort($tt);
var_dump( $tt );

echo "-- with table = HTML_SPECIALCHARS --\n";
$table = HTML_SPECIALCHARS;
$tt = get_html_translation_table($table, ENT_COMPAT, "UTF-8");
asort($tt);
var_dump( $tt );

echo "Done\n";
?>
