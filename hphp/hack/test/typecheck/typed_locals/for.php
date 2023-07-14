<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  let $i: int;
  for($i = 1; false; $i = $i + 1, $k = 1) {
    let $j: int = $i;
    $i = $i + 1;
    $j = $j * 2;
    let $k:int;
  }
  $i = 1;
  $j = 1;
}
function g(int $x): void {
  for($i = 1; false; $m = 1) {
    let $m:string;
  }

  for(; true;) {
    let $k:string;
  }
  $k = 1;
}
