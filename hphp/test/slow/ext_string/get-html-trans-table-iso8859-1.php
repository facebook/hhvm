<?hh


<<__EntryPoint>>
function main_get_html_trans_table_iso8859_1() :mixed{
$a = get_html_translation_table(HTML_ENTITIES, ENT_COMPAT, 'ISO-8859-1');

var_dump(ksort(inout $a));
var_dump($a);
}
