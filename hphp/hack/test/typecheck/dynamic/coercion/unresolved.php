<?hh // strict

function test(dynamic $x): void {
  if (true) {
    $y = 5;
  } else {
    $y = $x;
  }
  $y->someMethod(); // should fail, since $y could be int
}
