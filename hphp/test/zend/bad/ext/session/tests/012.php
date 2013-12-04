<?php
error_reporting(E_ALL);

### Absurd example, value of $_SESSION does not matter

session_id("abtest");
session_start();
$_SESSION["_SESSION"] = Array();
$_SESSION = "kk";

session_write_close();

### Restart to test for $_SESSION brokenness

session_start();
$_SESSION = "kk";
session_destroy();

print "I live\n";
?>