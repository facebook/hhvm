<?php

$a = 10;
 $b = array('test' => &$a);
 var_dump($b);
 $a = 20;
 var_dump($b);
