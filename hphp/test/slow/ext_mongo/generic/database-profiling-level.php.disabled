<?php
require_once __DIR__."/../utils/server.inc";
$a = mongo_standalone();
$d = $a->selectDb("phpunit");
$sp = $d->createCollection("system.profile", array('capped' => true, 'size' => 5000));

$prev = $d->setProfilingLevel(MongoDB::PROFILING_ON);
$level = $d->getProfilingLevel();
var_dump($level);

$prev = $d->setProfilingLevel(MongoDB::PROFILING_SLOW);
$level = $d->getProfilingLevel();
var_dump($prev);
var_dump($level);

$prev = $d->setProfilingLevel(MongoDB::PROFILING_OFF);
$level = $d->getProfilingLevel();
var_dump($prev);
var_dump($level);

$prev = $d->setProfilingLevel(MongoDB::PROFILING_OFF);
var_dump($prev);
?>