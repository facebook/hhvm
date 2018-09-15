<?php
 class T {
 static $a = array(A);
 }


<<__EntryPoint>>
function main_1134() {
define('A', 10);
 define('A', 20);
 var_dump(T::$a);
}
