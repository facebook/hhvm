<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');

$r->delete('Sicily');

$r->geoAdd('Sicily', '13.361389', '38.115556', 'Palermo', '15.087269', '37.502669', 'Catania');
var_dump($r->geoDist('Sicily', 'Palermo', 'Catania'));
var_dump($r->geoDist('Sicily', 'Palermo', 'Catania', 'mi'));
var_dump($r->geoDist('Sicily', 'Foo', 'Bar'));
var_dump($r->geoHash('Sicily', 'Palermo', 'Catania'));
var_dump($r->geoPos('Sicily', 'Palermo', 'Catania', 'NonExisting'));
var_dump($r->geoRadius('Sicily', '15', '37', '200', 'km', 'WITHDIST'));
var_dump($r->geoRadius('Sicily', '15', '37', '200', 'km', 'WITHCOORD'));
var_dump($r->geoRadius('Sicily', '15', '37', '200', 'km', 'WITHDIST', 'WITHCOORD'));
$r->geoAdd('Sicily', '13.583333', '37.316667', 'Agrigento');
var_dump($r->geoRadiusByMember('Sicily', 'Agrigento', '100', 'km'));
$r->delete('Sicily');
