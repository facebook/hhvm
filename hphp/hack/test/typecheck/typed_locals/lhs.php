<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  let $x:int = 1;
  list ($x, $y) = vec[1,2];
  let $z:vec<int> = vec[1,2,3];
  list ($x, $z[1]) = vec[1,2];
}
