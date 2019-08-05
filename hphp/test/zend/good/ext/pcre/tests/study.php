<?hh
<<__EntryPoint>> function main(): void {
  var_dump(
    preg_match_with_matches('/(?:(?:(?:(?:(?:(.))))))/  S', 'aeiou', &$dump),
  );
  var_dump($dump[1]);
  var_dump(
    preg_match_with_matches('/(?:(?:(?:(?:(?:(.))))))/', 'aeiou', &$dump),
  );
  var_dump($dump[1]);

  var_dump(
    preg_match_with_matches('/(?>..)((?:(?>.)|.|.|.|u))/S', 'aeiou', &$dump),
  );
  var_dump($dump[1]);

// try to trigger usual "match known text" optimization
  var_dump(preg_match_with_matches('/^aeiou$/S', 'aeiou', &$dump));
  var_dump($dump[0]);
  var_dump(preg_match_with_matches('/aeiou/S', 'aeiou', &$dump));
  var_dump($dump[0]);
}
