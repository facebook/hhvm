<?php

$m = new Memcached();
$m->addServer('localhost', '11211');

function dump_types($v, $k) {
    echo gettype($v) . "\n";
}

$keys = array(100, 'str');
array_walk($keys, 'dump_types');

$deleted = $m->deleteMulti($keys);
array_walk($keys, 'dump_types');
