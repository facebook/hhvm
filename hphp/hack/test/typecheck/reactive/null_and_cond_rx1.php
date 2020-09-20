<?hh

<<__Rx, __AtMostRxAsArgs>>
function my_test_function(
   <<__OnlyRxIfImpl(HH\Rx\Traversable::class)>> ?Traversable<mixed> $_
): void {}

<<__Rx>> function my_other_function(): void {
   my_test_function(null);
}
