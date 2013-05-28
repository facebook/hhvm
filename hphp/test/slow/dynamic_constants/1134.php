<?php

define('A', 10);
 class T {
 static $a = array(A);
 }
 define('A', 20);
 var_dump(T::$a);
