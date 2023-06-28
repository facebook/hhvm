<?hh


<<__EntryPoint>>
function main_1658() :mixed{
$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');
var_dump($a->subnode->subsubnode);
var_dump((string)($a->subnode->subsubnode));
}
