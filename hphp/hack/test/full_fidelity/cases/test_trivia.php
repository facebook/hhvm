<?hh

/**
 * Test case with various trivia such as end of line, fixme, whitespace
 * and comments
 */

function foo(dict<int,Plain,> $x): dict<
  int,
  Plain,
> {
  /* HH_FIXME: Some fixme */
  return $x;
}
