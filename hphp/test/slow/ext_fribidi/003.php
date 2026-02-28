<?hh <<__EntryPoint>> function main(): void {
error_reporting (E_ALL);
mb_internal_encoding("UTF-8");

$a = fribidi_log2vis(
  "THE dog 123 IS THE biggest",
  FRIBIDI_AUTO,
  FRIBIDI_CHARSET_CAP_RTL
);
$b = fribidi_log2vis(
  "THE dog 123 IS THE biggest",
  FRIBIDI_AUTO,
  FRIBIDI_CHARSET_UTF8
);
$c = "\xdb\x8c\xd9\x88\xd9\x86\xdb\x8c\xe2\x80\x8c\xda\xa9\xd8\xaf\x20\xd8".
  "\xa8\xd8\xb1\xd8\xa7\xdb\x8c\x20\xd9\x87\xd9\x85\xd9\x87";
$c = fribidi_log2vis($c, FRIBIDI_AUTO, FRIBIDI_CHARSET_UTF8);

var_dump(vec[$a, $b, $c]);
}
