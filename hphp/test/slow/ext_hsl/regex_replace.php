<?hh

function test_one($name, $haystack, $pattern, $replacement) {
  echo "-- $name --\n";
  echo "args: $haystack, $pattern, $replacement\n";
  echo "p_l_e before: " . preg_last_error() . "\n";
  list ($result, $error) =
    HH\Lib\_Private\_Regex\replace($haystack, $pattern, $replacement);
  echo "p_l_e after: " . preg_last_error() . "\n";
  echo "error: "; var_dump($error);
  var_dump($result);
}

<<__EntryPoint>> function test() {
  // Set low backtrack limit to trigger PREG_BACKTRACK_LIMIT_ERROR
  ini_set('pcre.backtrack_limit', 2);

  $cases = vec[
    // various errors
    vec['invalid pattern 1', 'lol', 'whut', 'the'],
    vec['invalid pattern 2', 'lol', '/whut', 'the'],
    vec['backtracking limit error', '0123456789', '/(..?)*\p{N}/', 'sup'],

    // various success cases
    vec['no match', 'abcd', '/de/', 'f'],
    vec['match at start', 'abcd', '/ab/', 'f'],
    vec['match in middle', 'abcd', '/bc/', 'f'],
    vec['match at end', 'abcd', '/cd/', 'f'],
    vec['match more than once', 'abcdefghi', '/[bdfh]/', 'x'],

    // now with backreferences
    vec['backreference style 1', 'foobar foobaz', '/foo(bar|baz)/', 'bar\1'],
    vec['backreference style 2', 'foobar foobaz', '/foo(bar|baz)/', 'bar$1'],
    vec['backreference style 3', 'foobar foobaz', '/foo(bar|baz)/', 'bar${1}'],
    vec[
      'multiple backreferences',
      'longcat, bigbird',
      '/(long|big)(cat|bird)/',
      '\2 \1',
    ],
    vec[
      'bad backreference numbers (currently) become empty strings',
      '12',
      '/(1)(2)/',
      '"$0" "$1" "$2" "$3" "$4" "${99}"',
    ],
  ];

  // cause preg_last_error() to be zero
  echo "== using preg_match to make preg_last_error zero ==\n";
  preg_match('/./', '0');

  // run all the cases
  foreach ($cases as $case) {
    test_one(...$case);
  }

  // cause preg_last_error() to be non-zero
  echo "== using preg_match to make preg_last_error non-zero ==\n";
  preg_match('/(..?)*\p{N}/', '0123456789');

  // run all the cases again to show we still don't mutate preg_last_error
  // and that preg_last_error doesn't affect the error we produce
  foreach ($cases as $case) {
    test_one(...$case);
  }
}
