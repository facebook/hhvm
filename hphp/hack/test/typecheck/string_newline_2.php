<?hh

function f(): int {
  /* Test that multiline double-quoted strings with curly braces don't mess up
   * line numbers in error messages. Curly braces are special because they
   * can potentially indicate interpolation, so we backtrack when we realize
   * that they are just plain characters. Make sure this backtracking handles
   * line numbers correctly. */
  $x = "hi {
  }";
  return "a";
}
