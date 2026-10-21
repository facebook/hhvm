<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>
function original(named int $b, int $a, named string $label = ''): int {
  return $a + $b;
}
function test(): int {
  /* HH_FIXME[4128] */
  return original(b = 2, 1, label = 'label');
}
