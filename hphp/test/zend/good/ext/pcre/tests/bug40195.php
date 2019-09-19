<?hh
<<__EntryPoint>> function main(): void {
  $m = null;
  var_dump(preg_match_with_matches('@^(/([a-z]*))*$@', '//abcde', inout $m));
  var_dump($m);
  var_dump(preg_match_with_matches('@^(/(?:[a-z]*))*$@', '//abcde', inout $m));
  var_dump($m);

  var_dump(preg_match_with_matches('@^(/([a-z]+))+$@', '/a/abcde', inout $m));
  var_dump($m);
  var_dump(preg_match_with_matches('@^(/(?:[a-z]+))+$@', '/a/abcde', inout $m));
  var_dump($m);
}
