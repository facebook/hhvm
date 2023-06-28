<?hh

function callback($match)
:mixed{
    var_dump($match);
    return $match[1].'/'.strlen($match['name']);
}
<<__EntryPoint>> function main(): void {
  $count = -1;
  var_dump(preg_replace_callback('|(?P<name>blub)|', callback<>, 'bla blub blah', -1, inout $count));

  $m = null;
  var_dump(preg_match_with_matches('|(?P<name>blub)|', 'bla blub blah', inout $m));
  var_dump($m);

  var_dump(preg_replace_callback('|(?P<1>blub)|', callback<>, 'bla blub blah', -1, inout $count));
}
