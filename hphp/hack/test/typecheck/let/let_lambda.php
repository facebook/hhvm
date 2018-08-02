<?hh // experimental

function foo(): void {
  let add_one = $x ==> $x + 1;
  let subtract_one : (function(int): int) = $x ==> $x - 1;
  let multiply : (function(int, int): int) = ($x, $y) ==> {
    let product = $x * $y;
    return product;
  };
  multiply(3, 2);
}
