<?hh

include (__DIR__ . '/redis.inc');
<<__EntryPoint>> function main(): void {
$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->setOption(Redis::OPT_SCAN, Redis::SCAN_RETRY);
$r->delete('sscan');

$r->sadd('sscan', 'member:one', 'member:two', 'member:three', 'member:four');

$cursor = null;
$ret = $r->sscan('sscan', $cursor);
sort($ret);
var_dump($ret);

$cursor = null;
$ret = $r->sscan('sscan', $cursor, 'member:t*');
sort($ret);
var_dump($ret);

$cursor = null;
$ret = $r->sscan('sscan', $cursor, 'nomember:t*');
sort($ret);
var_dump($ret);

$r->delete('sscan');
}
