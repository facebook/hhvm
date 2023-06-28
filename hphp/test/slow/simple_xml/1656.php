<?hh


<<__EntryPoint>>
function main_1656() :mixed{
$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><node a="b"><subnode attr1="value1" attr2="value2">test</subnode><subnode><subsubnode>test</subsubnode></subnode><test>v</test></node>');
var_dump($a->attributes());
var_dump((string)$a->subnode->offsetGet(0));
var_dump((string)$a->subnode->offsetGet(0)->offsetGet('attr1'));
var_dump((string)$a->subnode->offsetGet(1)->offsetGet('subsubnode'));
var_dump((string)$a->subnode->offsetGet(1)->subsubnode);
var_dump((string)$a->test);
var_dump($a->subnode->offsetGet(0)->attributes());
var_dump($a->subnode->offsetGet(1)->attributes());
var_dump($a->asXML());
var_dump((string)$a->addChild('newnode', 'newvalue'));
$a->addAttribute('newattr', 'newattrvalue');
var_dump($a->asXML());
var_dump($a->attributes());
var_dump((string)$a->newnode);
var_dump($a->getName());
var_dump($a->children()->subnode->offsetGet(0)->subsubnode);
$nodes = $a->xpath('//node/subnode');
var_dump((string)$nodes[1]->subsubnode);
$nodes = $nodes[1]->xpath('subsubnode');
var_dump((string)$nodes[0]);
}
