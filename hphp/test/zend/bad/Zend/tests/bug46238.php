<?php

class a {
        static function __callStatic($name, $arguments)
        {
                var_dump(array($name, $arguments));
        }
}

$a = 'a';
$b = '';

$a::$b($a);
$a::$b(array());
$a::$b(NULL);
$a::$b(1);
$a::$b();


$b = "\0";

$a::$b($a);
$a::$b(array());
$a::$b(NULL);
$a::$b(1);
$a::$b();

?>