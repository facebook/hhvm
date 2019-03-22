<?php
require "connect.inc";

$link = ldap_connect($host, $port);
var_dump($link);
echo "===DONE===\n";
