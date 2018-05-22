<?hh // strict

<<__Rx>>
function f3(): int {
  $a = <<__Rx>> () ==> {
    if (HH\Rx\IS_ENABLED) {
      return 1;
    } else {
      return 2;
    }
  };
  return $a();
}
