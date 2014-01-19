<?php

$x = new SimpleXMLElement('<foo/>');
 $x->addAttribute('attr', 'one');
 $x['attr'] = 'two';
 var_dump((string)$x['attr']);
 var_dump($x->asXML());
