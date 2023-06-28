<?hh


<<__EntryPoint>>
function main_1660() :mixed{
$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><node><subnode attr1="value1">test</subnode></node>');
var_dump((string)($a->subnode->offsetGet('attr1')));
}
