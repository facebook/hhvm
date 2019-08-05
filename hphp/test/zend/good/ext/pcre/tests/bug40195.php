<?hh
<<__EntryPoint>> function main(): void {
  var_dump(preg_match_with_matches('@^(/([a-z]*))*$@', '//abcde', &$m));
  var_dump($m);
  var_dump(preg_match_with_matches('@^(/(?:[a-z]*))*$@', '//abcde', &$m));
  var_dump($m);

  var_dump(preg_match_with_matches('@^(/([a-z]+))+$@', '/a/abcde', &$m));
  var_dump($m);
  var_dump(preg_match_with_matches('@^(/(?:[a-z]+))+$@', '/a/abcde', &$m));
  var_dump($m);
}
