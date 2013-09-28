<?php
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
if (!$ftp) die("Couldn't connect to the server");

var_dump(ftp_login($ftp, 'user', 'pass'));

var_dump(ftp_pwd($ftp));

var_dump(ftp_chdir($ftp, 'mydir'));
var_dump(ftp_pwd($ftp));

var_dump(ftp_chdir($ftp, '/xpto/mydir'));
var_dump(ftp_pwd($ftp));

var_dump(ftp_cdup($ftp));
var_dump(ftp_pwd($ftp));

var_dump(ftp_chdir($ftp, '..'));
var_dump(ftp_pwd($ftp));

var_dump(ftp_close($ftp));
?>