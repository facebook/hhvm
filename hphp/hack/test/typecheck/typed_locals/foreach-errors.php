<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(vec<int> $v): void {
  foreach ($v as $a) {
    $x = 15; // error: $x is bounded by string
    let $x: string = "foo";
  }

  let $y: string = "foo";
  foreach ($v as $a) {
    $y = 1; // error: $y is bounded by string
  }

  foreach ($v as $a) {
    let $z: string = "foo";
  }
  $z = 1; // error: $z is bounded by string
}

function g(vec<int> $v): void {
  let $a: string = "";
  foreach ($v as $a) {
  }
}

function h(vec<int> $v): void {
  let $a: int = 1;
  foreach ($v as $a) {
    $a = "";
  }
}
