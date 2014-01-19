<?php

$node = simplexml_load_string('<foo><bar>whoops</bar></foo>');
var_dump((string)$node);
