<?hh
<<__EntryPoint>> function main(): void {
  ini_set('pcre.backtrack_limit', 1);
  var_dump(@preg_match_all('/.*\p{N}/', '0123456789'));
  var_dump(preg_last_error() === PREG_BACKTRACK_LIMIT_ERROR);

  var_dump(preg_match_all('/\p{Nd}/', '0123456789'));
  var_dump(preg_last_error() === PREG_NO_ERROR);
}
