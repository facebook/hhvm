<?php
error_reporting(E_ALL);

$session_array = explode(";", @session_encode());
print "I live\n";
?>