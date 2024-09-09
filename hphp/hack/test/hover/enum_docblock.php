<?hh

enum E: string {
  /** My name is A */
  A = 'a'; // mlah
  /** My name is B */
  B = 'b'; // blah
  /** My name is C */
  C = 'c'; // slah
}

function main(): void {
  E::B;
  // ^ hover-at-caret
}
