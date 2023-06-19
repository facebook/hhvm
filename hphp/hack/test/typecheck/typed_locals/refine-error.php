<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function g(): void {
  let $x: int = 1;
  if ($x is arraykey) {
    $x = "";
  }
}
