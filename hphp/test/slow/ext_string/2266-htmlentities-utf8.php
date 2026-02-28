<?hh


<<__EntryPoint>>
function main_2266_htmlentities_utf8() :mixed{
var_dump(htmlentities("Europe’s", ENT_QUOTES, 'UTF-8'));
var_dump(htmlentities("77,39 €", ENT_QUOTES, 'UTF-8'));
}
