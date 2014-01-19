<?php
error_reporting(E_ALL);

session_id("abtest");
session_start();

?>
<a href="/link">
<?php
ini_set("session.use_trans_sid","1");
?>
<a href="/link">
<?php
ini_set("session.use_trans_sid","0");
?>
<a href="/link">
<?php
session_destroy();
?>