<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  $v = vec[1];
  let $i:int;
  foreach($v as $i) {
    let $j: int = $i;
    $i = $i + 1;
    $j = $j * 2;
    let $k:int;
  }
  $i = 1;
  $j = 1;
}

function g(): void {
  $v = dict["1" => 1];
  let $i1:string;
  let $i2:int;
  foreach($v as $i1 => $i2) {
    let $j: int = $i2;
    $i2 = $i2 + 1;
    let $k:string = $i1;
  }
}
