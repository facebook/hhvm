<?php

function so($a,$b) { return ord($a) - ord($b); }

$table = HTML_ENTITIES;
$tt = get_html_translation_table($table, ENT_COMPAT, "ISO-8859-1");
uksort( $tt, 'so' );
var_dump( count($tt) );
print_r( $tt );
echo "Done\n";

?>