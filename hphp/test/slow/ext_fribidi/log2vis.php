<?hh
// 4-byte UTF-8 codepoints exercise the max-width charset-conversion output.
<<__EntryPoint>>
function main(): mixed {
  $s = str_repeat("\xF0\x90\x80\x80", 4);
  $r = fribidi_log2vis($s, FRIBIDI_LTR, FRIBIDI_CHARSET_UTF8);
  var_dump($r === $s);
  var_dump(strlen($r));
}
