<?php

$uptime = HH\server_uptime();
var_dump($uptime);

$stopping = HH\server_is_stopping();
var_dump($stopping);

$health = HH\server_health_level();
var_dump($health);
