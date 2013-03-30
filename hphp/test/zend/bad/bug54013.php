<?php

class a
{
        function b($aaaaaaaa, $aaaaaaaa)
        {
                $params = func_get_args();
        }
}

$c = new a;
$c->b('waa?', 'meukee!');

$reflectionClass = new ReflectionClass($c);
$params = $reflectionClass->getMethod('b')->getParameters();

var_dump($params[0], $params[1]);

?>