<?hh

function hgoldstein(
  <<__Soft>> int $foobar,
): mixed {
  $_ = @hgoldstein(42);
  $_ = hgoldstein(95);
  return null;
}
