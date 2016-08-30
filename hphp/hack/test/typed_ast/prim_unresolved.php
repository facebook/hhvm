<?hh

function g(arraykey $x) : void {}

function foo(int $x) : void {
  if ($x > 5) {
    $y = 1;
  } else {
    $y = "hello";
  }
  g($y);
}
