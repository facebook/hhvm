<?php

namespace Foo;

function bar(&$ref) {
    $ref = 24;
}

$x = 42;
$ref =& $x;
\call_user_func('Foo\bar', $x);
var_dump($x);

$y = 42;
$ref =& $y;
call_user_func('Foo\bar', $y);
var_dump($y);

?>
