<?php

$a = array(50);
// This function is the easiest way to get hold of an ObjectData that isn't an
// Instance; this exercises getting and setting properties on these.
$it = hphp_get_iterator($a);
$it->blah = 2000;
var_dump($it->blah);
