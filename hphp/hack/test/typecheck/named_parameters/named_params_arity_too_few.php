<?hh

function fx(named int $x, named int $y = 1, string $z = ""): void {}
function fxyz1(named int $x, named int $y, named int $z, string $t): void {}
function fx_rest(named int $x, string ...$rest): void {}


function too_few_named(): void {
  fx("", y=1);
  fx(y=1, "");
  fxyz1("");
  fxyz1(y=1, x=1, "");
  fxyz1(x=1, y=1, "");
  fx_rest("", "", "", "", "");
}

function too_few_positional(): void {
  fxyz1(y=1, x=1, z=1);
}
