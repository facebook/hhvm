<?hh

function redundant_unsafe_casts(?string $str): void {
  HH\FIXME\UNSAFE_NONNULL_CAST($str); // Not redundant
  $str as nonnull;
  HH\FIXME\UNSAFE_NONNULL_CAST($str); // Redundant
}
