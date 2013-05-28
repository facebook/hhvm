<?php

$x = new SimpleXMLElement('<foo><bar>0</bar></foo>');
var_dump((bool)$x->bar);
