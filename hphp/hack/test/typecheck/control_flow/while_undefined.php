<?hh // strict

function f(): void {
  while (false) {
    $x = 10;
  }
  echo $x;
}
