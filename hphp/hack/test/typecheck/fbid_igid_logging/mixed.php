<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function sink(int $first, named int $instagram_id, named int $other): void {}
function test_mixed(): void {
  sink(make_fbid(), other = 456, instagram_id = make_igid());
  sink(make_fbid(), instagram_id = make_igid(), other = 456);
  sink(456, instagram_id = 123, other = 789);
}
