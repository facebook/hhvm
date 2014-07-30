<?hh

function f(): void {
  foreach (g() await as $x) {
  }
}
