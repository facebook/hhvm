<?php

$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');
var_dump((string)$node[0]);
