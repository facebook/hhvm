<?php

foreach (array(array(1,2), array(3,4)) as list($a, )) {
    var_dump($a);
}

$array = [['a', 'b'], 'c', 'd'];

foreach($array as list(list(), $a)) {
   var_dump($a); 
}

?>