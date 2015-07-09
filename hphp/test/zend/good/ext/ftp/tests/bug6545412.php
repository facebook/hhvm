<?php
require 'server.inc';

$ftp = ftp_connect('127.0.0.1', $port);
if (!$ftp) die("Couldn't connect to the server");

ftp_login($ftp, 'user', 'pass');

// Verify that you can safely access a Zend resource even after its been
// closed. See task #6545412.

var_dump($ftp);
ftp_close($ftp);
var_dump($ftp);
?>
