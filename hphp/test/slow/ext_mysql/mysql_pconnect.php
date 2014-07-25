<?php
require_once('connect.inc');

$conn = mysql_pconnect($host, $user, $passwd);
var_dump((bool)$conn);
