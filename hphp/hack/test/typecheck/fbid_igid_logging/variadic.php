<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function positional(int $first, int ...$rest): void {}
function named_sink(named int $fixed, named int...): void {}
function both(int $first, int ...$rest, named int...): void {}
function test_positional(): void {
  positional(456, make_igid(), make_fbid(), make_media_id());
}
function test_named_sink(): void {
  named_sink(fixed = 456, a = make_igid(), b = make_fbid(), c = make_media_id());
  named_sink(a = make_igid(), fixed = 456, b = make_fbid());
  named_sink(fixed = 456);
}
function test_both(): void {
  both(456, make_igid(), make_fbid(), a = make_igid(), b = make_fbid());
}
