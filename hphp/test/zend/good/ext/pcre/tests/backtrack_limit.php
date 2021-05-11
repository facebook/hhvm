<?hh

<<__EntryPoint>>
function main(): void {
  ini_set('pcre.backtrack_limit', 1);
  $error = null;
  $result = @preg_match_all_with_error('/.*\p{N}/', '0123456789', inout $error);
  var_dump($result);
  var_dump($error === PREG_BACKTRACK_LIMIT_ERROR);

  $error = null;
  $result = preg_match_all_with_error('/\p{Nd}/', '0123456789', inout $error);
  var_dump($result);
  var_dump($error === null);
}
