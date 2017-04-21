<?php

$m = new Memcached();
$m->addServer('localhost', '11211');

function matches_results($original, $compare)
{
    // get rid of $original keys that aren't in result
    $order = array_intersect_key($original, $compare);

    // fill values in original order
    $result = array_replace($order, $compare);

    return $original === $result;
}

$data = array(
    'foo' => 'foo-data',
    'bar' => 'bar-data',
    '2' => '2-data',
    '3' => '3-data',
);

$keys = array_keys($data);

$null = null;
$m->setMulti($data, 3600);

/* Check that all keys were stored */
var_dump(matches_results($data, $m->getMulti($keys)));

/* ---- same tests for byKey variants ---- */
$m->setMultiByKey("hi", $data, 3600);

var_dump(matches_results($data, $m->getMultiByKey('hi', $keys)));
