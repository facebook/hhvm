<?hh

function takes_int(int $_): void {}

function f(string $str): void {
  $o = new $str(); // Type error
  takes_int($o); // Terr passes as int
}
