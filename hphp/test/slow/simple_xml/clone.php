<?php

$node = simplexml_load_string(<<<EOF
<root one="1" two="2">
  <hello>world</hello>
</root>
EOF
);
$clone = clone $node;

var_dump((string)$clone->hello);
var_dump(count($clone->children()));
var_dump(count($clone->attributes()));

$clone->hello = 'test';
var_dump((string)$clone->hello);
var_dump((string)$node->hello);
