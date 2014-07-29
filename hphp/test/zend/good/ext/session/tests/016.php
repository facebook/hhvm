<?php
error_reporting(E_ALL);

session_start();
$HTTP_SESSION_VARS["test"] = 1;
session_write_close();
print "I live\n";
?>