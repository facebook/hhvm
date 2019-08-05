<?hh
<<__EntryPoint>> function main(): void {
var_dump(preg_quote('-oh really?'));

// make sure there's no regression in matching
  preg_match_with_matches('/[a\-c]+/', 'a---b', &$m);
  var_dump($m);

  preg_match_with_matches('/[a\-c]+/', 'a\-', &$m);
  var_dump($m);

  preg_match_with_matches("/a\-{2,}/", 'a----a', &$m);
  var_dump($m);

  preg_match_with_matches("/a\-{1,}/", 'a\----a', &$m);
  var_dump($m);
}
