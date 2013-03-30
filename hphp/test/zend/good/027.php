<?php

$a = 'b';
$b = 'c';
$c = 'strtoupper';

var_dump(${${$a}}('foo') == 'FOO');

$a = 'b';
$b = 'c';
$c = 'strtoupper';
$strtoupper = 'strtolower';

var_dump(${${++$a}}('FOO') == 'foo');

?>