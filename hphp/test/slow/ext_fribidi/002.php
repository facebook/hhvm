<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL ^ E_NOTICE);

echo FRIBIDI_CHARSET_UTF8."\n";
echo FRIBIDI_CHARSET_8859_8."\n";
echo FRIBIDI_CHARSET_8859_6."\n";
echo FRIBIDI_CHARSET_CP1255."\n";
echo FRIBIDI_CHARSET_CP1256."\n";
echo FRIBIDI_CHARSET_CAP_RTL."\n";

$charsets = fribidi_get_charsets();
foreach ($charsets as $code => $name) {
  print_r(fribidi_charset_info($code));
}
}
