<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
class Foo {
}
<<__Rx>>
function captures_mutable(
  <<__Mutable>> Foo $obj,
  <<__MaybeMutable>> Foo $obj2,
): void {
  $x = () ==> {
    $obj;
    $obj2;
  };
}
