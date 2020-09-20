<?hh
<<__EntryPoint>> function main(): void {
  $m = null;
  var_dump(preg_match_with_matches('/<.*>/', '<aa> <bb> <cc>', inout $m));
  var_dump($m);

  var_dump(preg_match_with_matches('/<.*>/U', '<aa> <bb> <cc>', inout $m));
  var_dump($m);

  var_dump(preg_match_with_matches('/(?U)<.*>/', '<aa> <bb> <cc>', inout $m));
  var_dump($m);
}
