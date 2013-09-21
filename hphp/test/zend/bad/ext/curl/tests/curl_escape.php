<?php
$str = "http://www.php.net/ ?!";

$a = curl_init();
$escaped = curl_escape($a, $str);
$original = curl_unescape($a, $escaped);
var_dump($escaped, $original);
var_dump(curl_unescape($a, 'a%00b'));
?>