<?hh


<<__EntryPoint>>
function main_1659() :mixed{
$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');
$v = $a->subnode->subsubnode->offsetGet('0');
var_dump(is_object($v) ? $v->__toString() : (string)$v);
var_dump($a->subnode->subsubnode->offsetGet(0)->__toString());
}
