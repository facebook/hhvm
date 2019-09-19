<?hh
/* Prototype  : proto int preg_match(string pattern, string subject [, array subpatterns [, int flags [, int offset]]])
 * Description: Perform a Perl-style regular expression match
 * Source code: ext/pcre/php_pcre.c
 * Alias to functions:
*/

<<__EntryPoint>> function main(): void {
$string = 'Hello, world. [*], this is \ a string';

  $match1 = null;
  var_dump(
    preg_match_with_matches('/^[hH]ello,\s/', $string, inout $match1),
  ); //finds "Hello, "
  var_dump($match1);

  $match2 = null;
  var_dump(preg_match_with_matches(
    '/l^o,\s\w{5}/',
    $string,
    inout $match2,
    PREG_OFFSET_CAPTURE,
  )); // tries to find "lo, world" at start of string
  var_dump($match2);

  $match3 = null;
  var_dump(
    preg_match_with_matches('/\[\*\],\s(.*)/', $string, inout $match3),
  ); //finds "[*], this is \ a string";
  var_dump($match3);

  $match4 = null;
  var_dump(preg_match_with_matches(
    '@\w{4}\s\w{2}\s\\\(?:\s.*)@',
    $string,
    inout $match4,
    PREG_OFFSET_CAPTURE,
    14,
  )); //finds "this is \ a string" (with non-capturing parentheses)
  var_dump($match4);

  $match5 = null;
  var_dump(
    preg_match_with_matches('/hello world/', $string, inout $match5),
  ); //tries to find "hello world" (should be Hello, world)
  var_dump($match5);
}
