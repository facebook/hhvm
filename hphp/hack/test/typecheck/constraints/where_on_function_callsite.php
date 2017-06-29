<?hh // strict


function foo<T> (T $x) : int where T = int {
  return $x;
}

function test() : void {
  $x = 5;
  $y = "hello";
  $z = foo($y); // error
}
