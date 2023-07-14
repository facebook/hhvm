<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function get_bool(): bool {
  return true;
}

function f(): void {

  while (get_bool()) {
    $x = 15;
    let $x: string = "foo"; // error: x already defined
  }

  let $y: string = "foo";
  while (get_bool()) {
    $y = 1; // error: $y is bounded by string
  }

  while (get_bool()) {
    let $z: string = "foo";
  }
  $z = 1; // error: $z is bounded by string
}

function g(): void {
  while (get_bool()) {
    $z = 1;
    continue;
    let $z: string = "foo"; // error: already defined
  }
  $z = 4;
}
