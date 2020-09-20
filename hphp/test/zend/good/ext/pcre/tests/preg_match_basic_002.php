<?hh
/* Prototype  : int preg_match  ( string $pattern  , string $subject  [, array &$matches  [, int $flags  [, int $offset  ]]] )
 * Description: Perform a regular expression match
 * Source code: ext/pcre/php_pcre.c
 */
<<__EntryPoint>> function main(): void {
$string = "My\nName\nIs\nStrange";
  $matches = null;
  preg_match_with_matches("/M(.*)/", $string, inout $matches);

  var_dump($matches);
echo "===Done===";
}
