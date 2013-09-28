<?php
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
if (!$ftp) die("Couldn't connect to the server");

var_dump(ftp_login($ftp, 'anonymous', 'IEUser@'));
// test for the correct behavior this time
var_dump(ftp_mkdir($ftp, 'CVS'));

?>