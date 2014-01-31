<?php

$node = simplexml_load_string('<root />');
var_dump(method_exists($node, 'saveXML'));
