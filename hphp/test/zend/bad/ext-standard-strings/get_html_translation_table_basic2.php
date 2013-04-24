<?php
/* Prototype  : array get_html_translation_table ( [int $table [, int $quote_style [, string charset_hint]]] )
 * Description: Returns the internal translation table used by htmlspecialchars and htmlentities
 * Source code: ext/standard/html.c
*/

/* Test get_html_translation_table() when table is specified as HTML_ENTITIES */

//set locale to en_US.UTF-8
setlocale(LC_ALL, "en_US.UTF-8");


echo "*** Testing get_html_translation_table() : basic functionality ***\n";

// Calling get_html_translation_table() with all arguments
// $table as HTML_ENTITIES and different quote style
echo "-- with table = HTML_ENTITIES & quote_style = ENT_COMPAT --\n";
$table = HTML_ENTITIES;
$quote_style = ENT_COMPAT;
$tt = get_html_translation_table($table, $quote_style, "UTF-8");
asort( $tt );
var_dump( $tt );

echo "-- with table = HTML_ENTITIES & quote_style = ENT_QUOTES --\n";
$quote_style = ENT_QUOTES;
$tt = get_html_translation_table($table, $quote_style, "UTF-8");
asort( $tt );
var_dump( $tt );

echo "-- with table = HTML_ENTITIES & quote_style = ENT_NOQUOTES --\n";
$quote_style = ENT_NOQUOTES;
$tt = get_html_translation_table($table, $quote_style, "UTF-8");
asort( $tt );
var_dump( $tt );


echo "Done\n";
?>