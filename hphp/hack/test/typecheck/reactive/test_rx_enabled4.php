<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f3()[rx]: int {
  $a = <<__Rx>> ()[rx] ==> {
    if (Rx\IS_ENABLED) {
      return 1;
    } else {
      return 2;
    }
  };
  return $a();
}
