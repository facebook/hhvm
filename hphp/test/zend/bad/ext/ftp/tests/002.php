<?php
$ssl = 1;
require 'server.inc';

$ftp = ftp_ssl_connect('127.0.0.1', $port);
if (!$ftp) die("Couldn't connect to the server");

var_dump(ftp_login($ftp, 'user', 'pass'));
var_dump(ftp_raw($ftp, 'HELP'));
var_dump(ftp_raw($ftp, 'HELP HELP'));

var_dump(ftp_close($ftp));
?>