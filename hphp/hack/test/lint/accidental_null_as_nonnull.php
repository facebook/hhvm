<?hh

function accidental_null_as_nonnull(shape(?'name' => string) $s): void {
  $s['name'] ?? null as nonnull
}
