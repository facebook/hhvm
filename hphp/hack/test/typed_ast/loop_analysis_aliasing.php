<?hh // strict

function loop(int $i) : void {
  $x = 2;
  while($i-- > 0) {
    // Loop analysis here sees different types for the expression "$x"
    // on different passes - The TAST sees only the overall union type.
    $y = $x;
    if ($i > 10) {
      $x = "hello";
    }
    $y; // This expression has a union type.
  }
}
