<?php

$a = -1; $b = -1; $c = -1; 
mbereg($a, $b, $c); 
var_dump($a, $b, $c); 

mberegi($a, $b, $c); 
var_dump($a, $b, $c); 

mbereg_search_init($a, $b, $c); 
var_dump($a, $b, $c);

echo "Done\n";
?>