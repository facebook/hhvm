<?php
$a = $b = $c = "oops";
parse_str("a=1&b=2&c=3");
var_dump($a, $b, $c);
?>