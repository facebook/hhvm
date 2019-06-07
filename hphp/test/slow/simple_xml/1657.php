<?hh


<<__EntryPoint>>
function main_1657() {
$a = new SimpleXMLElement('<?xml version="1.0" encoding="utf-8"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');
var_dump((array)($a->subnode->subsubnode));
var_dump((string)($a->subnode->subsubnode));
}
