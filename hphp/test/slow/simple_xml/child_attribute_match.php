<?php

$p = new SimpleXMLElement('<parent/>');
$c = $p->addChild('child', '123');
$c->addAttribute('attr', 'hi');

$cattrs = array();
foreach ($c->attributes() as $k => $v) {
  $cattrs[$k] = $v;
}

// $p->children()[0] should be the same node as $c
$pcattrs = array();
foreach ($p->children()[0]->attributes() as $k => $v) {
  $pcattrs[$k] = $v;
}

var_dump(count($cattrs) === count($pcattrs));
