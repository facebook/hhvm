<?php

$m = Map::fromArray(array('a' => 'foo'));
var_dump($m['a']);
var_dump($m->at('a'));
var_dump($m->get('a'));
var_dump($m->get('b'));
$sm = StableMap::fromArray(array('a' => 'foo'));
var_dump($sm['a']);
var_dump($sm->at('a'));
var_dump($sm->get('a'));
var_dump($sm->get('b'));
