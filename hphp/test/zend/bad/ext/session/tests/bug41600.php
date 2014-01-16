<?php

error_reporting(E_ALL);

session_id("abtest");
session_start();
?>
<a href="link.php?a=b">
<?php
session_destroy();
?>