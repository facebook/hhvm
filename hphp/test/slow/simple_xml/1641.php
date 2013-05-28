<?php

$x = new SimpleXMLElement('<foo/>');
$x->addChild('bar', 'whoops');
var_dump((string)$x);
