<?hh
<<__EntryPoint>> function main(): void {
  $match = null;
  var_dump(preg_match_with_matches('', '', inout $match, 0xfff));

  var_dump(preg_match_with_matches('/\d+/', '123 456 789 012', inout $match, 0, -8));
  var_dump($match);

  var_dump(
    preg_match_with_matches('/\d+/', '123 456 789 012', inout $match, 0, -500),
  );
  var_dump($match);

  var_dump(
    preg_match_all_with_matches('/\d+/', '123 456 789 012', inout $match, 0, -8),
  );
  var_dump($match);

var_dump(preg_match('/(?P<3>)/', ''));
}
