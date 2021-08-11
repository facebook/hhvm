<?hh

function takes_int(int $_): void {}

function f(): void {
  $d = new D(); // D doesn't exist
  takes_int($d); // Terr passes as an int
}
