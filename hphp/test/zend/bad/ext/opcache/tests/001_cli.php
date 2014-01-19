<?php
$config = opcache_get_configuration();
$status = opcache_get_status();
var_dump($config["directives"]["opcache.enable"]);
var_dump($config["directives"]["opcache.enable_cli"]);
var_dump($status["opcache_enabled"]);
?>