<?php
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
if (!$ftp) die("Couldn't connect to the server");

ftp_login($ftp, 'user', 'pass');
var_dump(ftp_size($ftp, 'largefile'));

ftp_close($ftp);
?>