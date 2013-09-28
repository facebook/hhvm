<?php

$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><node a="b"><subnode attr1="value1" attr2="value2">test</subnode><subnode><subsubnode>test</subsubnode></subnode><test>v</test></node>');
var_dump((array)$a->attributes());
var_dump((string)$a->subnode[0]);
var_dump((string)$a->subnode[0]['attr1']);
var_dump((string)$a->subnode[1]['subsubnode']);
var_dump((string)$a->subnode[1]->subsubnode);
var_dump((string)$a->test);
var_dump((array)$a->subnode[0]->attributes());
var_dump((array)$a->subnode[1]->attributes());
var_dump($a->asxml());
var_dump((string)$a->addchild('newnode', 'newvalue'));
$a->addattribute('newattr', 'newattrvalue');
var_dump($a->asxml());
var_dump((array)$a->attributes());
var_dump((string)$a->newnode);
var_dump($a->getname());
var_dump((array)$a->children()->subnode[0]->subsubnode);
$nodes = $a->xpath('//node/subnode');
var_dump((string)$nodes[1]->subsubnode);
$nodes = $nodes[1]->xpath('subsubnode');
var_dump((string)$nodes[0]);
