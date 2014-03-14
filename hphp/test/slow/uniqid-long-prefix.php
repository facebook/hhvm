<?php
$prefix = str_repeat('0!1@2#3$4%5^6&7*', 17); // 272 characters
$id = uniqid($prefix, true);
var_dump(strlen($id) > 272);
var_dump(strncmp($prefix, $id, strlen($prefix)) == 0);
var_dump(strpos($prefix, substr($id, strlen($prefix))) === false);
