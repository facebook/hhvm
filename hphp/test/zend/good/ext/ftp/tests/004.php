<?php
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
if (!$ftp) die("Couldn't connect to the server");

var_dump(ftp_login($ftp, 'user', 'pass'));

var_dump(ftp_systype($ftp));

/* some bogus usage */
var_dump(ftp_alloc($ftp, array()));
var_dump(ftp_cdup($ftp, 0));
var_dump(ftp_chdir($ftp, array()));
var_dump(ftp_chmod($ftp, 0666));
var_dump(ftp_get($ftp, 1234,12));
var_dump(ftp_close());
var_dump(ftp_connect('sfjkfjaksfjkasjf'));
var_dump(ftp_delete($ftp, array()));
var_dump(ftp_exec($ftp, array()));

var_dump(ftp_systype($ftp, 0));
var_dump(ftp_pwd($ftp, array()));

var_dump(ftp_login($ftp));
var_dump(ftp_login($ftp, 'user', 'bogus'));

var_dump(ftp_quit($ftp));
?>