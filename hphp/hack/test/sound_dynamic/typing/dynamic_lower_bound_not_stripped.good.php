<?hh

<<__SupportDynamicType>>
function g(int $cs): void {}

function f(bool $b, dynamic $d): void {
  $x = Vector{};
  if ($b) {
    $x[] = $d;
  } else {
    $x[] = 1;
  }
  $y = $x[0];
  g($y);
}
