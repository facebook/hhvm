<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function sink(named int $other, named int $instagram_id): void {}
function test_fbid(): void {
  sink(instagram_id = make_fbid(), other = 456);
  sink(other = 456, instagram_id = make_fbid());
}
function test_media_id(): void {
  sink(instagram_id = make_media_id(), other = 456);
  sink(other = 456, instagram_id = make_media_id());
}
