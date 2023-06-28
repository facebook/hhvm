<?hh


<<__EntryPoint>>
function main_simplexml_null() :mixed{
$date = date_create_immutable();
var_dump(dom_import_simplexml($date));
var_dump(simplexml_import_dom($date));
}
