<?php

require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
if (!$ftp) die("Couldn't connect to the server");

var_dump(ftp_login($ftp, 'user', 'pass'));


date_default_timezone_set('UTC');

$time = ftp_mdtm($ftp, "A");
echo date("F d Y H:i:s u",$time), PHP_EOL;

$time = ftp_mdtm($ftp, "B");
echo date("F d Y H:i:s u",$time), PHP_EOL;

$time = ftp_mdtm($ftp, "C");
echo date("F d Y H:i:s u",$time), PHP_EOL;

$time = ftp_mdtm($ftp, "D");
var_dump($time);

$time = ftp_mdtm($ftp, "19990929043300 File6");
echo date("F d Y H:i:s u",$time), PHP_EOL;

$time = ftp_mdtm($ftp, "MdTm 19990929043300 file6");
var_dump($time);

?>