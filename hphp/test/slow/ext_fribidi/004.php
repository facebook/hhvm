<?hh <<__EntryPoint>> function main(): void {
error_reporting (E_ALL);

$a = fribidi_log2vis(null,  FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);
$b = fribidi_log2vis(false, FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);
$c = fribidi_log2vis(true,  FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);
$d = fribidi_log2vis(0,     FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);
$e = fribidi_log2vis(1,     FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);
$f = fribidi_log2vis('',    FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);
$g = fribidi_log2vis("",    FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL);

var_dump(varray[$a, $b, $c, $d, $e, $f, $g]);
}
