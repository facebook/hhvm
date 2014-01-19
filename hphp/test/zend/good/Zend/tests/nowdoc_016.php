<?php 

$test = <<<'TEST'
TEST;

var_dump(strlen($test));

$test = <<<'TEST'
\
TEST;

var_dump(strlen($test));

$test = <<<'TEST'
\0
TEST;

var_dump(strlen($test));

$test = <<<'TEST'
\n
TEST;

var_dump(strlen($test));

$test = <<<'TEST'
\\'
TEST;

var_dump(strlen($test));

?>