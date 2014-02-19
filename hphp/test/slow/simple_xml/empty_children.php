<?php

$x = new SimpleXMLElement("<foo><x /></foo>");
var_dump($x->x->children()[0]);
