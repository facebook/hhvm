<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function get_bool(): bool {
  return true;
}

function g(): void {
  let $x: arraykey = 1;
  while (get_bool()) {
    $z = 1;
   continue;
    let $z: string = "foo";
  }
  $z = 4;
}

function f(): void {
  while (get_bool()) {
    let $z: string = "foo";
    break;
    $z = 1;
  }
}
