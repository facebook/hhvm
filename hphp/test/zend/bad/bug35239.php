<?php
$a = new stdClass; 
$a->x0 = new stdClass;
$a->x0->y0 = 'a';
$a->x0->y1 =& $a->x0;
$a->x0->y2 =& $a->x0;
$a->x0->y0 = 'b';
var_dump($a);
$a->x0->y1 = "ok\n";
echo $a->x0;
?>