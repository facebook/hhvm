<?php

$ar1 = array(10, 100, 100, 0);
 $ar2 = array(1, 3, 2, 4);
array_multisort($ar1, $ar2);
 var_dump($ar1, $ar2);
