<?hh
<<file:__EnableUnstableFeatures('ifc')>>


<<__Policied("PUBLIC")>>
function cipp_eval(<<__CanCall>> (function(): string) $f): void {
  try {
    $f();
  } catch (Exception $_) {
  }
}
