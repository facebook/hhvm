<?php
try { var_dump(socket_create_pair(AF_INET, null, null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$domain = 'unknown';
try { var_dump(socket_create_pair($domain, SOCK_STREAM, 0, &$sockets)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

var_dump(socket_create_pair(AF_INET, null, null, &$sockets));

var_dump(socket_create_pair(31337, null, null, &$sockets));

var_dump(socket_create_pair(AF_INET, 31337, 0, &$sockets));
