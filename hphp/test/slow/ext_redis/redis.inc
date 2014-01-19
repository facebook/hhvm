<?php

/**
 * Test configuration
 */
define('REDIS_HOST', getenv('REDIS_TEST_HOST'));
define('REDIS_PORT', getenv('REDIS_TEST_PORT')
                   ? (int)getenv('REDIS_TEST_PORT')
                   : Redis::DEFAULT_PORT);
define('REDIS_PASS', getenv('REDIS_TEST_PASS')
                   ? getenv('REDIS_TEST_PASS')
                   : null);

function NewRedisTestInstance($status = false) {
  $r = new Redis();
  $conn = $r->connect(REDIS_HOST, REDIS_PORT);
  if ($status) var_dump($conn);
  $authok = REDIS_PASS ? $r->auth(REDIS_PASS) : true;
  if ($status) var_dump($authok);
  return $r;
}

function GetTestKeyName($test, $rand = false) {
  $name = 'REDIS_TEST:' . preg_replace('/[^a-zA-Z0-9]/', '-', $test);
  if ($rand) $name .= ':' . rand(0,1000000);
  return $name;
}
