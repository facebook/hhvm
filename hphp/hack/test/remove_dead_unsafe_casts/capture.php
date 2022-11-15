<?hh

function f(): void {
  // Needs parantheses after removing UNSAFE_CAST
  2 * HH\FIXME\UNSAFE_CAST<mixed, int>(3 + 4);
}
