<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function get_bool(): bool {
  return true;
}


function f(): void {
  while (get_bool()) {
    let $z: string = "foo";
    $z = "1";
    break;
    $z = 1;
  }
}
