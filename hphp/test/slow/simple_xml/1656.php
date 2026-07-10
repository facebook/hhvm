<?hh


<<__EntryPoint>>
function main_1656() :mixed{
$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><node a="b"><subnode attr1="value1" attr2="value2">test</subnode><subnode><subsubnode>test</subsubnode></subnode><test>v</test></node>');
var_dump($a->attributes());
var_dump($a->subnode->offsetGet(0)->__toString());
var_dump($a->subnode->offsetGet(0)->offsetGet('attr1')->__toString());
$sub = $a->subnode->offsetGet(1)->offsetGet('subsubnode');
var_dump(is_object($sub) ? $sub->__toString() : (string)$sub);
var_dump($a->subnode->offsetGet(1)->subsubnode->__toString());
var_dump($a->test->__toString());
var_dump($a->subnode->offsetGet(0)->attributes());
var_dump($a->subnode->offsetGet(1)->attributes());
var_dump($a->asXML());
var_dump($a->addChild('newnode', 'newvalue')->__toString());
$a->addAttribute('newattr', 'newattrvalue');
var_dump($a->asXML());
var_dump($a->attributes());
var_dump($a->newnode->__toString());
var_dump($a->getName());
var_dump($a->children()->subnode->offsetGet(0)->subsubnode);
$nodes = $a->xpath('//node/subnode');
var_dump($nodes[1]->subsubnode->__toString());
$nodes = $nodes[1]->xpath('subsubnode');
var_dump($nodes[0]->__toString());
}
