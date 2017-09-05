<?hh // strict


const int X = 5;

const int Y = 2;
function test() : void {
  $y = X + 5;
}

namespace B {
  const int X = 7;
  function test() : void {
    $y = Y;
  }
  function test2() : void {
    $y = X + Y;
  }
  function test3() : void {
    $y = \B\X;
  }
}
