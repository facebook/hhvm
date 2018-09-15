<?hh // strict

function test(): void {
  if (true) {
    if (false) {
    }
    echo $x;
    $x = 1;
  } else {
    $x = 2;
  }
}
