<?php
$bogus = 1;
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
if (!$ftp) die("Couldn't connect to the server");

var_dump(ftp_login($ftp, 'anonymous', 'mail@example.com'));

var_dump(ftp_alloc($ftp, 400));
var_dump(ftp_cdup($ftp));
var_dump(ftp_chdir($ftp, '~'));
var_dump(ftp_chmod($ftp, 0666, 'x'));
var_dump(ftp_delete($ftp, 'x'));
var_dump(ftp_exec($ftp, 'x'));
var_dump(ftp_fget($ftp, STDOUT, 'x', 0));
var_dump(ftp_fput($ftp, 'x', fopen(__FILE__, 'r'), 0));
var_dump(ftp_get($ftp, 'x', 'y', 0));
var_dump(ftp_mdtm($ftp, 'x'));
var_dump(ftp_mkdir($ftp, 'x'));
var_dump(ftp_nb_continue($ftp));
var_dump(ftp_nb_fget($ftp, STDOUT, 'x', 0));
var_dump(ftp_nb_fput($ftp, 'x', fopen(__FILE__, 'r'), 0));
var_dump(ftp_systype($ftp));
var_dump(ftp_pwd($ftp));
var_dump(ftp_size($ftp, ''));
var_dump(ftp_rmdir($ftp, ''));

?>