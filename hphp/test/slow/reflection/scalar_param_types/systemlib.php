<?php

$rc = new ReflectionMethod('FilesystemIterator', 'setFlags');
$flags = $rc->getParameters()[0];
var_dump($flags->getClass());
var_dump($flags->getTypeHintText());
