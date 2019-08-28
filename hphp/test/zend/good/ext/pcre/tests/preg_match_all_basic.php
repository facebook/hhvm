<?hh
/*
* proto int preg_match_all(string pattern, string subject, [array subpatterns [, int flags [, int offset]]])
* Function is implemented in ext/pcre/php_pcre.c */
<<__EntryPoint>> function main(): void {
$string = 'Hello, world! This is a test. This is another test. \[4]. 34534 string.';
  $match1 = null;
  var_dump(preg_match_all_with_matches(
    '/[0-35-9]/',
    $string,
    inout $match1,
    PREG_OFFSET_CAPTURE | PREG_PATTERN_ORDER,
    -10,
  )); //finds any digit that's not 4 10 digits from the end(1 match)
  var_dump($match1);
  $match2 = null;
  var_dump(
    preg_match_all_with_matches(
      '/[tT]his is a(.*?)\./',
      $string,
      inout $match2,
      PREG_SET_ORDER,
    ),
  ); //finds "This is a test." and "This is another test." (non-greedy) (2 matches)
  var_dump($match2);
  $match3 = null;
  var_dump(
    preg_match_all_with_matches(
      '@\. \\\(.*).@',
      $string,
      inout $match3,
      PREG_PATTERN_ORDER,
    ),
  ); //finds ".\ [...]" and everything else to the end of the string. (greedy) (1 match)
  var_dump($match3);
  $match4 = null;
  var_dump(
    preg_match_all_with_matches('/\d{2}$/', $string, inout $match4),
  ); //tries to find 2 digits at the end of a string (0 matches)
  var_dump($match4);
  $match5 = null;
  var_dump(preg_match_all_with_matches(
    '/(This is a ){2}(.*)\stest/',
    $string,
    inout $match5,
  )); //tries to find "This is aThis is a [...] test" (0 matches)
  var_dump($match5);

// Test not passing in a subpatterns array.
var_dump(preg_match_all('/test/', $string));
var_dump(preg_match_all('/this isn\'t in the string/', $string));
var_dump(preg_match_all('/world/', $string));
var_dump(preg_match_all('/[0-9]/', $string));
}
