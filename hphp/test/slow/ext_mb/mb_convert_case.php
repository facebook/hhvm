<?php

$str = "mary had a Little lamb and she loved it so";
$str = mb_convert_case($str, MB_CASE_UPPER, "UTF-8");
var_dump($str);
$str = mb_convert_case($str, MB_CASE_TITLE, "UTF-8");
var_dump($str);
