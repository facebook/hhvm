<?php

$node = new SimpleXMLElement('<foo><bar name="value">whoops</bar></foo>');
var_dump((array)$node->bar);
