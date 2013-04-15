<?php

 $a = 10; $b = 20;function &foo(&$n, $p) { global $a; $n = 123; $p += 1;   var_dump('foo');return $a;}function &bar(&$n, $p) { global $b; $n = 456; $p += 2;   var_dump('bar');return $b;}fb_intercept('foo', 'fb_stubout_intercept_handler', 'bar');$n = 0; $d = 3; $c = &foo($n, $d); var_dump($c, $d); $c = 30;