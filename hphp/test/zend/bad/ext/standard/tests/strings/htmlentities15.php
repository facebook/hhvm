<?php
setlocale(LC_CTYPE, "ru_RU.koi8r", "ru_RU.KOI8-R");
$str = "���������";
var_dump($str, htmlentities($str, ENT_QUOTES, ''));
?>