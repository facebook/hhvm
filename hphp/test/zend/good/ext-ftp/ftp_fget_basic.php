<?php
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
if (!$ftp) die("Couldn't connect to the server");

var_dump(ftp_login($ftp, 'user', 'pass'));

//test simple text transfer
$fp = tmpfile();
var_dump(ftp_fget($ftp, $fp ,'a story.txt', FTP_ASCII));
fseek($fp, 0);
echo fgets($fp);

$postition = ftell($fp);
//test binary data transfer
var_dump(ftp_fget($ftp, $fp, 'binary data.bin', FTP_BINARY));
fseek($fp, $postition);
var_dump(urlencode(fgets($fp)));

//test non-existent file request 
ftp_fget($ftp, $fp ,'a warning.txt', FTP_ASCII);

//remove file
fclose($fp);
?>