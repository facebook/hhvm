<?hh


<<__EntryPoint>>
function main_1664() {
$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><?mso-application progid="Excel.Sheet"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');
var_dump((string)($a->subnode->subsubnode->offsetGet(0)));
}
