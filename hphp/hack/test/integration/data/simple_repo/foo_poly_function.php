<?hh

class Bigly {}
class Smol extends Bigly {}

function foo_poly_function(
  (function<<<__NoAutoBound>> T as Bigly super Smol>(T): T) $f,
): void {
  $g = $f;
}
