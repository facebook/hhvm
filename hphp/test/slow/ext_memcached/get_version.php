<?php

$memc = new Memcached();

$servers = [
  // Test both servers formats
  ['host' => 'localhost', 'port' => 11211, 'weight' => 50],
  ['localhost', 22222, 50] // Dummy port to check failure
];

$memc->addServers($servers);
var_dump($memc->getVersion());
