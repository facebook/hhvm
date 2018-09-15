<?hh // strict

function foo(): void {
  for (; false; $x = 1) {
  }
  echo $x;
}
