<?hh

include (__DIR__ . '/redis.inc');
<<__EntryPoint>> function main(): void {
$r = NewRedisTestInstance();
var_dump($r->echo("This is a test"));

$r->multi(Redis::MULTI);
var_dump($r->echo("This is a multi test") is Redis);
var_dump($r->exec());

$r->multi(Redis::PIPELINE);
var_dump($r->echo("This is a pipeline test") is Redis);
var_dump($r->exec());
}
