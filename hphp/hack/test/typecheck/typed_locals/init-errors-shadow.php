<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  let $a : string = "";
  let $a : arraykey = "";
}

function g(bool $b) : void {

  let $a : string = "";
  if ($b) {
    let $a : arraykey = "";
    let $c : int = 1;
  }
  let $c : arraykey = 1;
}
