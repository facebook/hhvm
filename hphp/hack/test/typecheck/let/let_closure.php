<?hh // experimental

function foo(): void {
  let add_one = function($x) {
    return $x + 1;
  };
  let subtract_one : (function(int): int) = function($x) {
    return $x - 1;
  };
  let multiply : (function(int, int): int) = function($x, $y) {
    let product = $x * $y;
    return product;
  };
  multiply(3, 2);
}
