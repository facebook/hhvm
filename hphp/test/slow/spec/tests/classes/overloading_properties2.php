<?php

// stripped-down version of overloading.php for inclusion in the spec

class Point { /*…*/ }       // has no public property "color"
$p = new Point(10, 15);

var_dump(isset($p->color));
$v = $p->color;         // get the dynamic property "color"
var_dump($v);

$p->color = "red";      // create/set the dynamic property "color"
$v = $p->color;         // get the dynamic property "color"
isset($p->color);       // test if (dynamic or not) "color" property exists
unset($p->color);       // remove the property "color"
