<?php

$x = new SimpleXMLElement('<foo><bar>345.234</bar></foo>');
var_dump((double)$x->bar);
