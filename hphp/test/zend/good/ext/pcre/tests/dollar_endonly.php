<?hh
<<__EntryPoint>> function main(): void {
  $m = null;
  var_dump(preg_match_all_with_matches('/^\S+.+$/', "aeiou\n", inout $m));
  var_dump($m);

  var_dump(preg_match_all_with_matches('/^\S+.+$/D', "aeiou\n", inout $m));
  var_dump($m);

  var_dump(preg_match_all_with_matches('/^\S+\s$/D', "aeiou\n", inout $m));
  var_dump($m);
}
