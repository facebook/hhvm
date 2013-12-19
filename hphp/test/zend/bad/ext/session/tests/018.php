<?php

error_reporting(E_ALL);

session_id("abtest");
session_start();
?>
<form accept-charset="ISO-8859-15, ISO-8859-1" action=url.php>
<?php
session_destroy();
?>