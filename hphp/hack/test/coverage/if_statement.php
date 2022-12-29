<?hh

function f(int $a, int $b):void {
  /* `Typing.expr` is called twice when typechecking an if condition, and this
   * test verifies that we are not double-counting
   * the number of expressions. */
  if ($a + $b) {}
}
