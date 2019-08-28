<?hh
<<__EntryPoint>> function main(): void {
  $m = null;
  var_dump(preg_match_with_matches('/a e i o u/', 'aeiou', inout $m));
  var_dump($m);

  var_dump(preg_match_with_matches('/a e i o u/x', 'aeiou', inout $m));
  var_dump($m);

  var_dump(preg_match_with_matches("/a e\ni\to\ru/x", 'aeiou', inout $m));
  var_dump($m);
}
