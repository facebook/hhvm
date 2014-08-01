<?php
$str = utf8_encode("\xe0\xe1");
var_dump(utf8_decode($str));
var_dump(utf8_decode(htmlspecialchars($str, ENT_COMPAT, "UTF-8")));
?>