<?hh

include (__DIR__ . '/redis.inc');
<<__EntryPoint>> function main(): void {
$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('A');

$r->zAdd('A', 1, 'one');
$r->zAdd('A', 2, 'two');
$r->zAdd('A', 3, 'three');
var_dump($r->zScore('A', 'one'));
var_dump($r->zScore('A', 'three'));

$r->delete('A');
}
