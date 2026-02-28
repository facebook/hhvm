<?hh

<<file: __EnableUnstableFeatures('typed_local_variables')>>

function test(): void {
  // This is fine
  $g = (vec<_> $x) ==> $x;
  // This should be an error
  let $a: vec<_> = vec[0];
  $f = (int $x): int ==> {
    // Wildcard in typed local is illegal, even under lambda
    let $y: vec<_> = vec[$x];
    // But this should be legal
    let $h: (function(vec<int>): vec<int>) = (vec<_> $x) ==> $x;
    return $y[0];
  };
}
