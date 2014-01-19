<?php

$argv[] = __DIR__."/phpunit_mock.inc";
$arvc += 1;
$_SERVER['argv'] = $argv;
$_SERVER['argc'] = $argc;

include __DIR__."/phpunit.phar";
