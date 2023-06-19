<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function get_bool(): bool {
  return true;
}

function f(): void {

  while (get_bool()) {
    $x = 15; // error: $x is bounded by string
    let $x: string = "foo";
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
  let $x: arraykey = 1;
  while (get_bool()) {
    /* The following is an error because if we don't go through the loop, after
     * the loop $x = 1, but the string bound from inside the loop also applies.
     */
    let $x: string = "";
  }
}
