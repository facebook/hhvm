<?php

$node = new SimpleXMLElement('<foo>whoops</foo>');
var_dump((string)$node);
