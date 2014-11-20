<?php 
define('REDIS_HOST', getenv('REDIS_TEST_HOST'));
define('REDIS_PORT', getenv('REDIS_TEST_PORT')
                   ? (int)getenv('REDIS_TEST_PORT')
                   : Redis::DEFAULT_PORT);
define('REDIS_PASS', getenv('REDIS_TEST_PASS')
                   ? getenv('REDIS_TEST_PASS')
                   : null);
	
function NewRedisTestInstance($status = false) {
$expecting = array(
			'port' => 6379,
			'timeout' => 0,
			'persistent' => true,
			'database' => 0
		);
  $r = new Redis();
  $persistentId = $expecting['port'] . $expecting['timeout'] . $expecting['database'];
  $conn = $r->pconnect(REDIS_HOST, REDIS_PORT,$expecting['timeout'],$persistentId);
  
  if ($status) var_dump($conn);
  $authok = REDIS_PASS ? $r->auth(REDIS_PASS) : true;
  if ($status) var_dump($authok);
  return $r;
}



$r = NewRedisTestInstance(true);
if($r)
	echo true;


