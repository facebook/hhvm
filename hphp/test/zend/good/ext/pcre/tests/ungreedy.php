<?hh
<<__EntryPoint>> function main(): void {
  var_dump(preg_match_with_matches('/<.*>/', '<aa> <bb> <cc>', &$m));
  var_dump($m);

  var_dump(preg_match_with_matches('/<.*>/U', '<aa> <bb> <cc>', &$m));
  var_dump($m);

  var_dump(preg_match_with_matches('/(?U)<.*>/', '<aa> <bb> <cc>', &$m));
  var_dump($m);
}
