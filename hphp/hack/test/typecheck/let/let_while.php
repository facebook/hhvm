<?hh // experimental

function foo(int $x): void {
  while ($x < 0) {
    let y : int = $x;
    $x++;
  }
  echo y; // error
}
