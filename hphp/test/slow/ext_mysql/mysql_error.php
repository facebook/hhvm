<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(@mysql_select_db('nonexistentdb', $con));
var_dump(mysql_error($conn));
