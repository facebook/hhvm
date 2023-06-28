<?hh


<<__EntryPoint>>
function main_1659() :mixed{
$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');
var_dump((string)($a->subnode->subsubnode->offsetGet('0')));
var_dump((string)($a->subnode->subsubnode->offsetGet(0)));
}
