<?php

$to_exec = PHP_BINARY.' --php -n '.__DIR__.'/argv.inc foo bar';
var_dump($to_exec);
system($to_exec);
