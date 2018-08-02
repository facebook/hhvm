<?hh // experimental

function foo(int $x): void {
  do {
    let y : int = $x;
    $x++;
  } while ($x < 0);
  echo y; // error
}
