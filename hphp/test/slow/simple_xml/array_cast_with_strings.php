<?php

$node = simplexml_load_string(<<<EOF
<config>
<hello>world</hello>
  <how>
    <are>you</are>
  </how>
  <hi></hi>
  <ho />
</config>
EOF
);
$node->addChild('test', 'best');
$nodes = (array)$node;
var_dump($nodes);
$arr[$nodes['hello']] = 1;
var_dump($arr);
