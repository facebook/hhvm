<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  list ($x, $y) = vec[1,2];
  let $x:int = 1; // error
  tuple ($x1, $y1) = tuple (1, 2);
  let $x1:int = 1; // error
}
