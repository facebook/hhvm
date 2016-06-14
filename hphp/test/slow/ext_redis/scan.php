<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
global $prefix;
$prefix = GetTestKeyName(__FILE__) . ':';
$r->setOption(Redis::OPT_PREFIX, $prefix);
$r->setOption(Redis::OPT_SCAN, Redis::SCAN_RETRY);
$ret = $r->delete('scan');
$ret = $r->mset(array('key:one' => 'one', 'key:two' => 'two',
               'key:three' => 'three','key:four' => 'four'));

/*
 * The PHP5 extension we're patterning after doesn't try to turn
 * OPT_PREFIX into a pattern when doing scan(), so scan returns
 * global state. Filter it ourselves.
 */
function performScan($fn){
	$cursor = null;
	$returns = array();
	while (($retval = $fn($cursor))){
		foreach ($retval as $key){
			global $prefix;
			if (strstr($key, $prefix) !== false){
				$returns []= substr($key, strlen($prefix));
			}
		}
	}
	// Normalize return order
	sort($returns);
	return $returns;
}

try {
	$ret = performScan(function(&$cursor) use($r){
		// Scan without patterns.
		return $r->scan($cursor);
	});
	var_dump($ret);

	$ret = performScan(function(&$cursor) use($r){
		// catch key:two and key:three
		global $prefix;
		return $r->scan($cursor, $prefix . 'key:t*');
	});
	var_dump($ret);

	$ret = performScan(function(&$cursor) use($r){
		// nothing.
		global $prefix;
		return $r->scan($cursor, $prefix . 'nokey:t*');
	});
	var_dump($ret);
} finally {
	$r->delete('key:one');
	$r->delete('key:two');
	$r->delete('key:three');
	$r->delete('key:four');
}
