<?hh
<<__EntryPoint>> function main(): void {
  $match = null;
  var_dump(
    preg_match_all_with_matches('/(.)x/', 'zxax', inout $match, PREG_PATTERN_ORDER),
  );
  var_dump($match);

  var_dump(
    preg_match_all_with_matches('/(.)x/', 'zxyx', inout $match, PREG_SET_ORDER),
  );
  var_dump($match);

  var_dump(
    preg_match_all_with_matches('/(.)x/', 'zxyx', inout $match, PREG_OFFSET_CAPTURE),
  );
  var_dump($match);

  var_dump(preg_match_all_with_matches(
    '/(.)x/',
    'zxyx',
    inout $match,
    PREG_SET_ORDER | PREG_OFFSET_CAPTURE,
  ));
  var_dump($match);
}
