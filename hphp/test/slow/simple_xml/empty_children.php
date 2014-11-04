<?php

$x = new SimpleXMLElement("<foo><x /></foo>");
var_dump($x->x->children()[0]);
var_dump(empty($x->x) ? "empty" : "not empty");
