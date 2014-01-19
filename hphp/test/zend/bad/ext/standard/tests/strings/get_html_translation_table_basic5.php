<?php
echo "*** Testing get_html_translation_table() : basic functionality/HTML 5 ***\n";

echo "-- with table = HTML_ENTITIES, ENT_COMPAT --\n";
$table = HTML_ENTITIES;
$tt = get_html_translation_table($table, ENT_COMPAT | ENT_HTML5, "UTF-8");
asort( $tt );
var_dump( count($tt) );
print_r( $tt );

echo "-- with table = HTML_ENTITIES, ENT_QUOTES --\n";
$table = HTML_ENTITIES;
$tt = get_html_translation_table($table, ENT_QUOTES | ENT_HTML5, "UTF-8");
var_dump( count($tt) );

echo "-- with table = HTML_ENTITIES, ENT_NOQUOTES --\n";
$table = HTML_ENTITIES;
$tt = get_html_translation_table($table, ENT_NOQUOTES | ENT_HTML5, "UTF-8");
var_dump( count($tt) );

echo "-- with table = HTML_SPECIALCHARS, ENT_COMPAT --\n";
$table = HTML_SPECIALCHARS; 
$tt = get_html_translation_table($table, ENT_COMPAT, "UTF-8");
asort( $tt );
var_dump( count($tt) );
print_r( $tt );

echo "-- with table = HTML_SPECIALCHARS, ENT_QUOTES --\n";
$table = HTML_SPECIALCHARS;
$tt = get_html_translation_table($table, ENT_QUOTES | ENT_HTML5, "UTF-8");
asort( $tt );
var_dump( $tt );

echo "-- with table = HTML_SPECIALCHARS, ENT_NOQUOTES --\n";
$table = HTML_SPECIALCHARS;
$tt = get_html_translation_table($table, ENT_NOQUOTES | ENT_HTML5, "UTF-8");
asort( $tt );
var_dump( $tt );


echo "Done\n";
?>