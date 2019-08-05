<?hh
<<__EntryPoint>> function main(): void {
  var_dump(preg_match_with_matches('/a e i o u/', 'aeiou', &$m));
  var_dump($m);

  var_dump(preg_match_with_matches('/a e i o u/x', 'aeiou', &$m));
  var_dump($m);

  var_dump(preg_match_with_matches("/a e\ni\to\ru/x", 'aeiou', &$m));
  var_dump($m);
}
