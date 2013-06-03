<?php
require_once("hphpd.php");
error_log("In ".$_SERVER['PHP_SELF']);
interrupt('web_request');
echo "interrupt done";
