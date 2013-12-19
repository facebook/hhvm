<?php
echo mb_internal_encoding()."\n";

$ngchars = array('能','表','蚕','ソ');
$str = '元禄養蚕會社詐欺表現能力表示噂免停暴力貼付構文圭子予知饅頭ソファー';
var_dump(strlen($str));
var_dump(mb_strlen($str));