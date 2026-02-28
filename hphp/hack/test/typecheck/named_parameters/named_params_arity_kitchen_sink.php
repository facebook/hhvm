<?hh
//
function fx(named int $x, named int $y = 1, string $z = ""): void {}
function fxyz1(named int $x, named int $y, named int $z, string $t): void {}
function fx_rest(named int $x, string ...$rest): void {}

function kitchen_sink(): void {
  fxyz1();
  fx();
  fxyz1(extra1=1, extra2=2);
  fxyz1("", extra1=1, "", extra2=2);
}
