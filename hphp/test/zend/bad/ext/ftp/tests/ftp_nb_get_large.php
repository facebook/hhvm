<?php
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
ftp_login($ftp, 'user', 'pass');
if (!$ftp) die("Couldn't connect to the server");

$local_file = dirname(__FILE__) . DIRECTORY_SEPARATOR . "ftp_nb_get_large.txt";
touch($local_file);
ftp_nb_get($ftp, $local_file, 'fget_large.txt', FTP_BINARY, 5368709119);
$fp = fopen($local_file, 'r');
fseek($fp, 5368709119);
var_dump(fread($fp, 1));
var_dump(filesize($local_file));
fclose($fp);
?>
<?php
@unlink(dirname(__FILE__) . DIRECTORY_SEPARATOR . "ftp_nb_get_large.txt");
?>