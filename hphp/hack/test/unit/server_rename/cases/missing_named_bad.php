<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>
function original(named int $b): void {}
function test(): void {
  /* HH_FIXME[4128] */
  original(1);
}
