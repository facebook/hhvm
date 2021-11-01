<?hh
/**
 * Regression test: newlines in strings used to mess up line numbers in error
 * messages.
 */
function test(string $x): string {
  $_ = "$x
  ";
  $_ = "$x
  ";
  $_ = "$x
  ";
  $_ = "$x
  ";
  $_ = "$x
  ";

  return 4;
}
