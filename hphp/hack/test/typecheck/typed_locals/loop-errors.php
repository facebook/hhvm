<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function get_bool() : bool {
  return true;
}

function f() : void {

  while (get_bool()) {
   $x = 15;
   let $x : string = "foo";
  }

  let $y : string = "foo";
  while (get_bool()) {
    $y = 1;
  }

  while (get_bool()) {
  let $z : string = "foo";
  }
  $z = 1;
}
