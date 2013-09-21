<?php
$a = "‚ ‚¢‚¤‚¦‚¨";
$b = $a;
mb_convert_variables("EUC-JP", "Shift_JIS", $b);
debug_zval_dump($a);
debug_zval_dump($b);
unset($a);
unset($b);

$a = "‚ ‚¢‚¤‚¦‚¨";
$b = &$a;
mb_convert_variables("EUC-JP", "Shift_JIS", $b);
debug_zval_dump($a);
debug_zval_dump($b);
unset($a);
unset($b);

$a = "‚ ‚¢‚¤‚¦‚¨";
$b = array($a);
$c = $b;
mb_convert_variables("EUC-JP", "Shift_JIS", $c);
debug_zval_dump($b);
debug_zval_dump($c);
unset($a);
unset($b);
unset($c);

$a = "‚ ‚¢‚¤‚¦‚¨";
$b = array(&$a);
$c = $b;
mb_convert_variables("EUC-JP", "Shift_JIS", $c);
debug_zval_dump($b);
debug_zval_dump($c);
unset($a);
unset($b);
unset($c);

$a = "‚ ‚¢‚¤‚¦‚¨";
$b = array($a);
$c = &$b;
mb_convert_variables("EUC-JP", "Shift_JIS", $c);
debug_zval_dump($b);
debug_zval_dump($c);
unset($a);
unset($b);
unset($c);

$a = "‚ ‚¢‚¤‚¦‚¨";
$b = array(&$a);
$c = &$b;
mb_convert_variables("EUC-JP", "Shift_JIS", $c);
debug_zval_dump($b);
debug_zval_dump($c);
unset($a);
unset($b);
unset($c);

$a = array(array("‚ ‚¢‚¤‚¦‚¨"));
$b = $a;
$c = $b;
mb_convert_variables("EUC-JP", "Shift_JIS", $c);
debug_zval_dump($b);
debug_zval_dump($c);
unset($a);
unset($b);
unset($c);
?>