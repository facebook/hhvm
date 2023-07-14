<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(int $x): void {
$j = 1;
for($i = 1; $i < $x; $m = 1) {  // error: wrong type
    let $j:int = $i;     // error: already defined
    let $i:int = $i + 1; // error: already defined
    $j = $j * 2;
    let $k:int;
    let $m:string;
  }
  $k = ""; // error: wrong type
}
