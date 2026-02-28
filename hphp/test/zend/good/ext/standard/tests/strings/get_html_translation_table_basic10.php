<?hh

function so($a,$b) :mixed{ return ord($a) - ord($b); }
<<__EntryPoint>> function main(): void {
$table = HTML_ENTITIES;
$tt = get_html_translation_table($table, ENT_COMPAT, "ISO-8859-1");
uksort(inout $tt, so<> );
var_dump( count($tt) );
print_r( $tt );
echo "Done\n";
}
