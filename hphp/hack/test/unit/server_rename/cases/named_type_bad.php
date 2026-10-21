<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>
class C {
  public static function original(named int $b): void {}
}
function test(): void {
  /* HH_FIXME[4128] */
  C::original(b = 'wrong');
}
