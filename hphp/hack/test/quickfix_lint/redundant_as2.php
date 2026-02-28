<?hh

function redundant_as2(int $i): void {
  $i as nonnull; // redundant
  $i = 1 === 2 ? $i : null;
  $i as nonnull; // essential
  $i as nonnull; // redundant
}
