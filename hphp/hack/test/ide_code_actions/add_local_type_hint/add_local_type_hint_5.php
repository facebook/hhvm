<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  $the_variable = 3;
  // Offer refactroing to add type hint for $x
  $x = /*range-start*/$the_variable/*range-end*/;
}
