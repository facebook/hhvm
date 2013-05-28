<?php

$a = 1;
 $b = &$a;
 $c = $b;
 $b = 2;
 var_dump($a);
 var_dump($c);
