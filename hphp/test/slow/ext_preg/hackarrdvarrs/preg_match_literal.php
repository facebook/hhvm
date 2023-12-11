<?hh


<<__EntryPoint>>
function main_preg_match_literal() :mixed{
$subject = "Quick brown fox";
var_dump(preg_match("#^Quick#", $subject));
var_dump(preg_match("/fox\$/", $subject));
var_dump(preg_match("/fox\$/", "foxfox"));
var_dump(preg_match("%fox\$%", "x"));
var_dump(preg_match("%bcd%", "abcdef"));
var_dump(preg_match("%^bcd%", "abcdef"));

$matches = dict[];
  var_dump(preg_match_with_matches("%bcd%", "abcdbcdef", inout $matches));
  var_dump(count($matches));
var_dump(utf8_decode($matches[0]));

  var_dump(preg_match_with_matches(
    "%bcd%",
    "abcdbcdef",
    inout $matches,
    $flags = PREG_OFFSET_CAPTURE,
  ));
  var_dump(utf8_decode($matches[0][0]));
var_dump($matches[0][1]);
// Matching the end of the string
  var_dump(preg_match_with_matches("#bcd\$#", "abcdbcdbcd", inout $matches));
  var_dump(count($matches));
// var_dump(utf8_decode($matches[0])); /* Missing byte? */

// Multiple subpatterns
  var_dump(preg_match_with_matches("#bcd#", "abcdbcdbcd", inout $matches));
  var_dump(count($matches));
var_dump(utf8_decode($matches[0]));

// preg_match_all should still work
  var_dump(preg_match_all_with_matches("#bcd#", "abcdbcdbcd", inout $matches));
  var_dump(count($matches));
var_dump(count($matches[0]));
var_dump(($matches[0][0]));
var_dump(($matches[0][1]));
var_dump(($matches[0][2]));

// Special characters
var_dump(preg_match("#\.#", "abcde"));
var_dump(preg_match("#\.#", "abc.de"));
var_dump(preg_match("# #", "abcde"));
var_dump(preg_match("# #", "abc de"));
var_dump(preg_match("#_#", "abcde"));
var_dump(preg_match("#_#", "abc_de"));
var_dump(preg_match("#-#", "abcde"));
var_dump(preg_match("#-#", "abc-de"));
var_dump(preg_match("#:#", "abcde"));
var_dump(preg_match("#:#", "abc:de"));

// Pattern is longer than subject
var_dump(preg_match("#foooobar$#", "bar"));
var_dump(preg_match("#foooobar$#", "foo"));

// Long pattern (shouldn't segfault)
var_dump(preg_match("#foooooooooooooooooooooooooooooooooooooooooooooooooooooo".
  "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo#", "foo"));

// Case sensitivity
var_dump(preg_match("#foO#", "FOOOO"));
var_dump(preg_match("#foO#i", "FOOOO"));

// Initial substring match should fail if $ used
var_dump(preg_match("#^foo$#", "fooooooooooo"));

// Escaped literals
var_dump(preg_match("/^\d\.$/", "4."));
var_dump(preg_match("/^\d\.$/", "44"));
var_dump(preg_match("/^foo\\n$/", "foo\n"));

// Trailing backslash
var_dump(preg_match('/abc\\/', 'abc\\'));
var_dump(preg_match('/abc\\\\/', 'abc\\'));

// Empty pattern should match everything.
var_dump(preg_match('//', 'abc'));

// Offsets
  var_dump(preg_match_with_matches(
    '/def/',
    'abcdef',
    inout $matches,
    PREG_OFFSET_CAPTURE,
    2,
  ));
  var_dump($matches);
  var_dump(preg_match_with_matches(
    '/def/',
    'abcdef',
    inout $matches,
    PREG_OFFSET_CAPTURE,
    4,
  ));
  var_dump($matches);
  var_dump(
    preg_match_with_matches(
      '//',
      'abcdef',
      inout $matches,
      PREG_OFFSET_CAPTURE,
      3
  ));
  var_dump($matches);
  var_dump(preg_match_with_matches(
    '/^def/',
    'abcdef',
    inout $matches,
    PREG_OFFSET_CAPTURE,
    3,
  ));
  var_dump($matches);
  var_dump(preg_match_with_matches(
    '/def$/',
    'abcdef',
    inout $matches,
    PREG_OFFSET_CAPTURE,
    3,
  ));
  var_dump($matches);
}
