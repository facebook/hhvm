<?php

$a = <<<NOWDOC
"'\t
NOWDOC;
var_dump($a);
$a = <<<'NOWDOC'
"'\t
NOWDOC;
var_dump($a);
$a = <<<"NOWDOC"
"'\t
NOWDOC;
var_dump($a);
