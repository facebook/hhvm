<?hh

use namespace HH\Lib\Regex;

function f(Regex\Pattern $pattern): void {
  echo($pattern);
}

f("hello\n");

$patterns = vec[
  tuple('/foo(bar)?/', 'foo'),
  tuple('/foo(?<bar>bar)?/', 'foo'),
  tuple('/foo(?<bar>bar)?(baz)?/', 'foo'),
  tuple('/foo(?<bar>bar)?(baz)?/', 'foobar'),
  tuple('/foo(?<bar>bar)?(baz)?/', 'foobaz'),
  tuple('/foo(?<bar>bar)?(baz)?/', 'foobarbaz'),
];

foreach ($patterns as list($pattern, $text)) {
  $matches = [];
  preg_match($pattern, $text, &$matches);
  printf("---\nRE: %s\nIn: %s\nPHP:\n", $pattern, $text);
  var_dump($matches);
  preg_match($pattern, $text, &$matches, PREG_FB__PRIVATE__HSL_IMPL);
  printf("HSL:\n", $pattern, $text);
  var_dump($matches);
}
