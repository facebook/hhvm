<?hh

use namespace HH\Lib\Regex;

const PATTERNS = vec[
  tuple('/foo(bar)?/', 'foo'),
  tuple('/foo(?<bar>bar)?/', 'foo'),
  tuple('/foo(?<bar>bar)?(baz)?/', 'foo'),
  tuple('/foo(?<bar>bar)?(baz)?/', 'foobar'),
  tuple('/foo(?<bar>bar)?(baz)?/', 'foobaz'),
  tuple('/foo(?<bar>bar)?(baz)?/', 'foobarbaz'),
];

function test(int $flags): void {
  foreach (PATTERNS as list($pattern, $text)) {
    $matches = dict[];
    preg_match_with_matches($pattern, $text, inout $matches, $flags);
    printf("---\nRE: %s\nIn: %s\nPHP:\n", $pattern, $text);
    \var_dump($matches);
    preg_match_with_matches(
      $pattern,
      $text,
      inout $matches,
      PREG_FB__PRIVATE__HSL_IMPL | $flags,
    );
    printf("HSL:\n", $pattern, $text);
    \var_dump($matches);
  }
}

<<__EntryPoint>>
function main(): void {
  printf("Matches Only:\n");
  test(0);
  printf("Matches with Offsets:\n");
  test(PREG_OFFSET_CAPTURE);
}
