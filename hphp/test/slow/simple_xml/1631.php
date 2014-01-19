<?php

$x = new SimpleXMLElement('<foo><bar></bar></foo>');
var_dump((bool)$x->bar);
