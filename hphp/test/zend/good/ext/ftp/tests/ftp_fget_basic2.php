<?php
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
ftp_login($ftp, 'user', 'pass');
if (!$ftp) die("Couldn't connect to the server");

$local_file = dirname(__FILE__) . DIRECTORY_SEPARATOR . "ftp_fget_basic2.txt";
file_put_contents($local_file, 'ASCIIFoo');
$handle = fopen($local_file, 'a');

var_dump(ftp_fget($ftp, $handle, 'fgetresume.txt', FTP_ASCII, FTP_AUTORESUME));
var_dump(file_get_contents($local_file));
?>
<?php error_reporting(0); ?>
<?php
@unlink(dirname(__FILE__) . DIRECTORY_SEPARATOR . "ftp_fget_basic2.txt");
?>