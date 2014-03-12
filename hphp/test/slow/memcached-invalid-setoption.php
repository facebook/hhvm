<?php
$memcache = new \Memcached();
var_dump($memcache->setOption(-1, 'option'));
