<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function g(vec<int> $v): void {
  let $x : arraykey = "";
  foreach ($v as $x) {
    $z = 1;
    // let $x:int = 1; currently an error
    continue;
    let $z: string = "foo";
  }
  $z = 4;
}

function f(vec<int> $v): void {
  foreach ($v as $x) {
    let $z: string = "foo";
    break;
    $z = 1;
  }
}
