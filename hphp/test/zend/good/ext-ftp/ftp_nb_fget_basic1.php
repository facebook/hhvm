<?php
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
ftp_login($ftp, 'user', 'pass');
if (!$ftp) die("Couldn't connect to the server");
ftp_set_option($ftp, FTP_AUTOSEEK, false);

$local_file = dirname(__FILE__) . DIRECTORY_SEPARATOR . "ftp_nb_fget_basic1.txt";
$handle = fopen($local_file, 'w');

var_dump(ftp_nb_fget($ftp, $handle, 'fget.txt', FTP_ASCII, FTP_AUTORESUME));
var_dump(file_get_contents($local_file));
?><?php
@unlink(dirname(__FILE__) . DIRECTORY_SEPARATOR . "ftp_nb_fget_basic1.txt");
?>
