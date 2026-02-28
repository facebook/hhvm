<?hh


<<__EntryPoint>>
function main_get_html_trans_table_utf8() :mixed{
$a = get_html_translation_table(HTML_ENTITIES, ENT_QUOTES);

var_dump(ksort(inout $a));
var_dump($a);
}
