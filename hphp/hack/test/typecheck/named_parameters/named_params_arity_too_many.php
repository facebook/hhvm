<?hh

function fx(named int $x, named int $y = 1, string $z = ""): void {}
function fxyz1(named int $x, named int $y, named int $z, string $t): void {}
function fx_rest(named int $x, string ...$rest): void {}

function too_many_named(): void {
  fx(x=1, extra1=1);
  fx(x=1, y=1, extra1=1);
  fxyz1(y=1, x=1, "", z=1, extra1=1, extra2=1);
  fxyz1(x=1, y=1, z=1, extra1=1, "");
  fx_rest(x=1, "", "", "", extra1=1, extra2=1);
}

function too_many_positional(): void {
  fx("", y=1, "", "", x=1);
  fxyz1(x=1, y=1, z=1, "", "");
}
