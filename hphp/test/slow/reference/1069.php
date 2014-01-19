<?php

$a = array();
 $b = 10;
 $a[] = &$b;
 $b = 20;
 var_dump($a);
