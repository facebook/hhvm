<?php

$m = new Memcached();

$m->setOption(Memcached::OPT_BINARY_PROTOCOL, true);
$m->addServer('localhost', '11211');

$key = uniqid('touch_binary');

$m->set($key, 'test', time() + 86400);

$m->get($key);
echo "GET: " . $m->getResultMessage() . PHP_EOL;

$m->touch($key, time() + 86400);
echo "TOUCH: " . $m->getResultMessage() . PHP_EOL;

$m->touch($key, time() + 86400);
echo "TOUCH: " . $m->getResultMessage() . PHP_EOL;

$m->get($key);
echo "GET: " . $m->getResultMessage() . PHP_EOL;

$m->get($key);
echo "GET: " . $m->getResultMessage() . PHP_EOL;

echo "DONE" . PHP_EOL;
