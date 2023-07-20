<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  $v = vec["1"];
  let $i:int;
  foreach($v as $i) { // error
  }
}

function g(): void {
  $v = dict["1" => 1];
  foreach($v as $i1 => $i2) {
    let $i1:string; // error
    let $i2:int; // error
  }
}
