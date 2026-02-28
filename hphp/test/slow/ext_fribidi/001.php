<?hh <<__EntryPoint>> function main(): void {
error_reporting (E_ALL ^ E_NOTICE);

$b = "\xe1\xec\xe4\x20\x31\x39\x38\x20\x66\x6f\x6f\x20\xe1\xec\xe4\x20\x42"
  ."\x41\x52\x20\x31\x32";
$c = "\x46\x72\x69\x20\xe1\xe9\xe3\xe9\x20\xe9\xe0\xe0\xe0\x20\x62\x6c\x61"
  ."\x20\x31\x32\x25\x20\x62\x6c\x61";

$a = fribidi_log2vis(
  "THE dog 123 IS THE biggest",
  FRIBIDI_AUTO,
  FRIBIDI_CHARSET_CAP_RTL
);

$b = bin2hex(fribidi_log2vis($b, FRIBIDI_RTL, FRIBIDI_CHARSET_8859_8));
$b = "\\x" . substr(chunk_split($b, 2, "\\x"), 0, -2);
$c = bin2hex(fribidi_log2vis($c, FRIBIDI_AUTO, FRIBIDI_CHARSET_CP1255));
$c = "\\x" . substr(chunk_split($c, 2, "\\x"), 0, -2);

var_dump(vec[$a, $b, $c]);
}
