<?php

$mem = new Memcached();
$mem->addServer("localhost", 1234);
var_dump(count($mem->getServerList()));
$mem->resetServerList();
var_dump($mem->getServerList());
