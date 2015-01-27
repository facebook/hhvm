<?php
define('REDIS_HOST', getenv('REDIS_TEST_HOST'));
define('REDIS_PORT', getenv('REDIS_TEST_PORT')
                   ? (int)getenv('REDIS_TEST_PORT')
                   : Redis::DEFAULT_PORT);
define('REDIS_PASS', getenv('REDIS_TEST_PASS') !== null
                   ? getenv('REDIS_TEST_PASS')
                   : null);

function NewRedisTestInstance() {
  $expecting = array(
    'timeout'  => 0,
    'database' => 0
  );
  $r = new Redis();
  $persistentId = REDIS_PORT . $expecting['timeout'] . $expecting['database'];
  $conn = $r->pconnect(
    REDIS_HOST, REDIS_PORT, $expecting['timeout'], $persistentId);
  var_dump($conn);
  $authok = REDIS_PASS ? $r->auth(REDIS_PASS) : true;
  var_dump($authok);
  return $r;
}

$r = NewRedisTestInstance();
if ($r) echo true;
