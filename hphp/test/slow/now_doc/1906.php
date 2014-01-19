<?php

$b = 'bad';
$a = <<<'NOWDOC'
$b
NOWDOC;
var_dump($a);
$a = <<<"NOWDOC"
$b
NOWDOC;
var_dump($a);
$a = <<<NOWDOC
$b
NOWDOC;
var_dump($a);
