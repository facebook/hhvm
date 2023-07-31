<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  list ($x, $y) = vec[1,2];
  let $x:int = 1; // error
}
