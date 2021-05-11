<?hh

function test_one($name, $haystack, $pattern, $replacement) {
  echo "-- $name --\n";
  echo "args: $haystack, $pattern, $replacement\n";
  list ($result, $error) =
    HH\Lib\_Private\_Regex\replace($haystack, $pattern, $replacement);
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

  // run all the cases
  foreach ($cases as $case) {
    test_one(...$case);
  }
}
