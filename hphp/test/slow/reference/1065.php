<?php

$a = 1;
 $b = &$a;
 $c = 2;
 $b = $c;
 $c = 5;
 var_dump($a);
 var_dump($b);
 var_dump($c);
