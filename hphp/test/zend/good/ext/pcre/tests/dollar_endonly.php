<?hh
<<__EntryPoint>> function main(): void {
  var_dump(preg_match_all_with_matches('/^\S+.+$/', "aeiou\n", &$m));
  var_dump($m);

  var_dump(preg_match_all_with_matches('/^\S+.+$/D', "aeiou\n", &$m));
  var_dump($m);

  var_dump(preg_match_all_with_matches('/^\S+\s$/D', "aeiou\n", &$m));
  var_dump($m);
}
