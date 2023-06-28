<?hh

function test_one($name, $haystack, $pattern, $offset) :mixed{
  echo "-- $name --\n";
  echo "args: $haystack, $pattern, $offset\n";
  list ($matches, $error) =
    HH\Lib\_Private\_Regex\match($haystack, $pattern, inout $offset);
  echo "error: "; var_dump($error);
  echo "offset: $offset\n";
  var_dump($matches);
}

<<__EntryPoint>> function test() :mixed{
  // Set low backtrack limit to trigger PREG_BACKTRACK_LIMIT_ERROR
  ini_set('pcre.backtrack_limit', 1);

  $cases = vec[
    // various errors
    vec['invalid pattern 1', 'lol', 'whut', 0],
    vec['invalid pattern 2', 'lol', '/whut', 0],
    vec['backtracking limit error', '0123456789', '/.*\p{N}/', 0],
    vec['offset does not change on error', 'lol', 'whut', 2],

    // no match
    vec['no match (pattern)', 'lol', '/whut/', 0],
    vec['no match (offset)', 'lol', '/lol/', 1],
    vec['no match (offset from end)', 'lol', '/lol/', -1],
    vec['no match (offset past the end)', 'lol', '/o/', 5],

    // match
    vec['match part at start', 'lol', '/lo/', 0],
    vec['match whole', 'lol', '/lol/', 0],
    vec['match after start moves offset', 'lol', '/ol/', 0],
    vec['offset before beginning still matches', 'lol', '/o/', -5],
  ];

  // simulate walking through with offset to get every match
  // offset for next step is output offset of previous step + match length
  $haystack = 'abbaabbbbaaaba';
  $pattern = '/ab/';
  $cases[] = vec['multi-step 1', $haystack, $pattern, 0];
  $cases[] = vec['multi-step 2', $haystack, $pattern, 2];
  $cases[] = vec['multi-step 3', $haystack, $pattern, 6];
  $cases[] = vec['multi-step 4', $haystack, $pattern, 13];

  // run all the cases
  foreach ($cases as $case) {
    test_one(...$case);
  }
}
