<?php
$a = array(1, 2);
$c = $a;
var_dump(substr_replace($a, 1, 1, $c));
?>