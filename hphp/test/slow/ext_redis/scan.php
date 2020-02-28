<?hh

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
ExtRedisScan::$prefix = GetTestKeyName(__FILE__) . ':';
$r->setOption(Redis::OPT_PREFIX, ExtRedisScan::$prefix);
$r->setOption(Redis::OPT_SCAN, Redis::SCAN_RETRY);
$ret = $r->delete('scan');
$ret = $r->mset(darray['key:one' => 'one', 'key:two' => 'two',
               'key:three' => 'three','key:four' => 'four']);

/*
 * The PHP5 extension we're patterning after doesn't try to turn
 * OPT_PREFIX into a pattern when doing scan(), so scan returns
 * global state. Filter it ourselves.
 */
function performScan($fn){
	$cursor = null;
	$returns = varray[];
	while (($retval = $fn(inout $cursor))){
		foreach ($retval as $key){

			if (strstr($key, ExtRedisScan::$prefix) !== false){
				$returns []= substr($key, strlen(ExtRedisScan::$prefix));
			}
		}
	}
	// Normalize return order
	sort($returns);
	return $returns;
}

try {
	$ret = performScan(function(inout $cursor) use($r){
		// Scan without patterns.
		return $r->scan(inout $cursor);
	});
	var_dump($ret);

	$ret = performScan(function(inout $cursor) use($r){
		// catch key:two and key:three

		return $r->scan(inout $cursor, ExtRedisScan::$prefix . 'key:t*');
	});
	var_dump($ret);

	$ret = performScan(function(inout $cursor) use($r){
		// nothing.

		return $r->scan(inout $cursor, ExtRedisScan::$prefix . 'nokey:t*');
	});
	var_dump($ret);
} finally {
	$r->delete('key:one');
	$r->delete('key:two');
	$r->delete('key:three');
	$r->delete('key:four');
}

abstract final class ExtRedisScan {
  public static $prefix;
}
