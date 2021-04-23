<?hh

<<__EntryPoint>> function main(): void {
  error_reporting (E_ALL);

  var_dump(fribidi_log2vis('', FRIBIDI_AUTO, FRIBIDI_CHARSET_CAP_RTL));
}
