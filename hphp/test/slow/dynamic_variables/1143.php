<?php

$a = 'test';
 $b = 'a';
 $c = &$$b;
 $c = 10;
 var_dump($a);
