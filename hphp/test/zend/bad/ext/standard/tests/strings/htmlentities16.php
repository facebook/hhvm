<?php
$str = "\x88\xa9\xf0\xee\xf1\xea\xee\xf8\xed\xfb\xe9";
var_dump(bin2hex($str), bin2hex(htmlentities($str, ENT_QUOTES, '')));
var_dump(htmlentities($str, ENT_QUOTES | ENT_HTML5, ''));
?>
===DONE===