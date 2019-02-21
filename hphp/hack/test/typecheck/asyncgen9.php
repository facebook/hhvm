<?hh // partial

function f(): void {
  foreach (g() await as $x) {
  }
}
