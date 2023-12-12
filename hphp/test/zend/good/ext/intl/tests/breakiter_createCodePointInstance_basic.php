<?hh
<<__EntryPoint>>
function main_entry(): void {
  ini_set("intl.error_level", E_WARNING);
  ini_set("intl.default_locale", "pt_PT");

  $text = "\xe0\xb8\x95\xe0\xb8\xb1\xe0\xb8\xa7\xe0\xb8\xad\xe0\xb8\xa2\xe0\xb9\x88\xe0\xb8\xb2\xe0\xb8\x87\xe0\xb8\x82\xe0\xb9\x89\xe0\xb8\xad\xe0\xb8\x84\xe0\xb8\xa7\xe0\xb8\xb2\xe0\xb8\xa1";

  $codepoint_it = IntlBreakIterator::createCodePointInstance();
  var_dump(get_class($codepoint_it));
  $codepoint_it->setText($text);

  print_r(iterator_to_array($codepoint_it));
  echo "==DONE==";
}
