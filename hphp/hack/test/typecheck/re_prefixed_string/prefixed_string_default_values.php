<?hh

function test_with_default(
  HH\Lib\Regex\Pattern<shape(...)> $pattern = re'/default/',
): void {
  echo $pattern;
}
