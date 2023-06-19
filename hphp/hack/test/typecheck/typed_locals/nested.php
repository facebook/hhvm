<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function get_bool(): bool {
  return true;
}

function g(): void {
  let $a: arraykey = "";
  while (get_bool()) {
    if (get_bool()) {
      continue;
    } else {
      let $a: int = 1;
    }
  }
  $a = "";
}
