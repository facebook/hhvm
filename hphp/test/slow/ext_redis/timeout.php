<?hh
// https://github.com/facebook/hhvm/issues/3094
include (__DIR__ . '/redis.inc');
<<__EntryPoint>> function main(): void {
try {
  $r = newRedisTestInstance();
  $st = microtime(true);
  $r->setOption(Redis::OPT_READ_TIMEOUT, 2);
  var_dump($r->getOption(Redis::OPT_READ_TIMEOUT));
  $val = $r->blpop('mykey',0);
  $et = microtime(true);
} catch(Exception $ex) {
  echo "ex: " . $ex->getMessage() . "\n";
  $et = microtime(true);
}
echo "Took: " . ($et-$st) . " to get the key\n"; // Should be 2.x seconds
}
