<?php
mb_encoding_aliases();
$list = mb_encoding_aliases("ASCII");
sort($list);
var_dump($list);
var_dump(mb_encoding_aliases("7bit"));
var_dump(mb_encoding_aliases("8bit"));
?>