<?php
$dt = DateTime::createFromFormat('O', '+0800');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('P', '+08:00');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('O', '-0800');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('P', '-08:00');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('[O]', '[+0800]');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('[P]', '[+08:00]');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('[O]', '[-0800]');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('[P]', '[-08:00]');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('O', 'GMT+0800');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('P', 'GMT+08:00');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('O', 'GMT-0800');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('P', 'GMT-08:00');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('[O]', '[GMT+0800]');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('[P]', '[GMT+08:00]');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('[O]', '[GMT-0800]');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('[P]', '[GMT-08:00]');
var_dump($dt->getOffset());

$dt = DateTime::createFromFormat('O', 'invalid');
var_dump($dt);
?>