<?php

$a = 1;
 $c = $b = &$a;
 $b = 2;
 var_dump($a);
 var_dump($c);
