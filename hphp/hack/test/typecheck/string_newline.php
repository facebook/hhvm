<?hh // partial

/**
 * Regression test: newlines in strings used to mess up line numbers in error
 * messages.
 */
function test(?varray<int> $a) {
  $x = "
    ";
  $x = "
    ";
  $x = "
    ";
  $x = "
    ";
  $x = "
    ";
  $x = "
    ";
  $a[0];
}
