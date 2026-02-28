<?hh

function hgoldstein(
  @int $foobar,
): mixed {
  $_ = @hgoldstein(42);
  $_ = hgoldstein(95);
  return null;
}
