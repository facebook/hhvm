<?hh

function NewRedisTestInstance() {
  $expecting = darray[
    'timeout'  => 0,
    'database' => 0
  ];
  $r = new Redis();
  $redis_host = getenv('REDIS_TEST_HOST');
  $redis_port = getenv('REDIS_TEST_PORT')
    ? (int)getenv('REDIS_TEST_PORT')
    : Redis::DEFAULT_PORT;
  $persistentId = $redis_port . $expecting['timeout'] . $expecting['database'];
  $conn = $r->pconnect(
    $redis_host, $redis_port, $expecting['timeout'], $persistentId);
  var_dump($conn);
  $redis_pass = getenv('REDIS_TEST_PASS');
  $authok = $redis_pass ? $r->auth($redis_pass) : true;
  var_dump($authok);
  return $r;
}
<<__EntryPoint>> function main(): void {
$r = NewRedisTestInstance();
if ($r) echo true;
}
