<?hh

function f(): void {
  for (; false; $x = 10) {
  }
  echo $x;
}
