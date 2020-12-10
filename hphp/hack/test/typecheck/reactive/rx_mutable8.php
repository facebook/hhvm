<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f(): void {
  $a = HH\Rx\mutable(Map {});
  $b = HH\Rx\mutable(ImmMap {});
  $c = HH\Rx\mutable(Set {});
  $d = HH\Rx\mutable(Vector {});
  $e = HH\Rx\mutable(ImmVector {});
  $f = HH\Rx\mutable(ImmSet {});
  $g = HH\Rx\mutable(Pair { 1, 2 });
}
