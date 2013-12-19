<?php
echo mb_internal_encoding()."\n";

$ngchars = array('能','表','蚕','ソ');
$str = '元禄養蚕會社詐欺表現能力表示噂免停暴力貼付構文圭子予知饅頭ソファー';
$converted_str = mb_convert_encoding($str, 'Shift_JIS');
mb_regex_encoding('Shift_JIS');
foreach($ngchars as $c) {
	$c = mb_convert_encoding($c, 'Shift_JIS');
	$replaced = mb_convert_encoding(ereg_replace($c, '!!', $converted_str), mb_internal_encoding(), 'Shift_JIS');
	var_dump(strpos($replaced, '!!'));
}
?>