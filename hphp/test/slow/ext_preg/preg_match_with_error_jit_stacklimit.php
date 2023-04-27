<?hh

<<__EntryPoint>> function main(): void {
  // The essential features of $pattern are that pcre jits it and that it
  // causes ridiculous amounts of backtracking with branches on the $subject
  $pattern = re"/a(.|\n)+?b/";
  $subject = 'a' . str_repeat('x', 1024 * 16) . 'b';
  $error = -1;
  var_dump(preg_match_with_error($pattern, $subject, inout $error));
  var_dump($error === PREG_JIT_STACKLIMIT_ERROR);
}
