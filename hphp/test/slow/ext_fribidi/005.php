<?hh <<__EntryPoint>> function main(): void {
error_reporting (E_ALL);

$a = fribidi_log2vis("ABC def", FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);
$b = fribidi_log2vis("ABC def", FRIBIDI_LTR,  FRIBIDI_CHARSET_CAP_RTL);
$c = fribidi_log2vis("ABC def", FRIBIDI_RTL,  FRIBIDI_CHARSET_CAP_RTL);
$d = fribidi_log2vis("ABC def", FRIBIDI_WLTR, FRIBIDI_CHARSET_CAP_RTL);
$e = fribidi_log2vis("ABC def", FRIBIDI_WRTL, FRIBIDI_CHARSET_CAP_RTL);
var_dump(vec[$a, $b, $c, $d, $e]);

$a = fribidi_log2vis("abc DEF", FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);
$b = fribidi_log2vis("abc DEF", FRIBIDI_LTR,  FRIBIDI_CHARSET_CAP_RTL);
$c = fribidi_log2vis("abc DEF", FRIBIDI_RTL,  FRIBIDI_CHARSET_CAP_RTL);
$d = fribidi_log2vis("abc DEF", FRIBIDI_WLTR, FRIBIDI_CHARSET_CAP_RTL);
$e = fribidi_log2vis("abc DEF", FRIBIDI_WRTL, FRIBIDI_CHARSET_CAP_RTL);
var_dump(vec[$a, $b, $c, $d, $e]);

$a = fribidi_log2vis("123 456", FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);
$b = fribidi_log2vis("123 456", FRIBIDI_LTR,  FRIBIDI_CHARSET_CAP_RTL);
$c = fribidi_log2vis("123 456", FRIBIDI_RTL,  FRIBIDI_CHARSET_CAP_RTL);
$d = fribidi_log2vis("123 456", FRIBIDI_WLTR, FRIBIDI_CHARSET_CAP_RTL);
$e = fribidi_log2vis("123 456", FRIBIDI_WRTL, FRIBIDI_CHARSET_CAP_RTL);
var_dump(vec[$a, $b, $c, $d, $e]);
}
