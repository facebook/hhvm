<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(!empty(mysql_get_host_info()));
