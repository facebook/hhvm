<?hh
<<file:__EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>


function take_err1((function (named int $x, named string $y): void) $_): void {}
function f_err1(named int $x): void {}
function test_err1(): void {
  take_err1(f_err1<>); // ERROR: Missing required named parameter: y
}


function take_err2((function (named int $x): void) $_): void {}
function f_err2(named int $x, named string $y): void {}
function test_err2(): void {
  take_err2(f_err2<>); // ERROR: Unexpected named parameter: y
}


function take_err3((function (named int $x): void) $_): void {}
function f_err3(named int $wrong): void {}
function test_err3(): void {
  take_err3(f_err3<>); // ERROR: Missing x, Unexpected wrong
}


function take_err4((function (named int $a, named string $b): void) $_): void {}
function f_err4(named int $a, named string $wrong): void {}
function test_err4(): void {
  take_err4(f_err4<>); // ERROR: Missing b, Unexpected wrong
}


function take_err5((function (named int $x, named string $y): void) $_): void {}
function f_err5(named int $p, named string $q): void {}
function test_err5(): void {
  take_err5(f_err5<>); // ERROR: Missing x and y, Unexpected p and q
}


function take_err6((function (named int $a, named string $b, named bool $c): void) $_): void {}
function f_err6(named int $a, named float $d): void {}
function test_err6(): void {
  take_err6(f_err6<>); // ERROR: Missing b and c, Unexpected d
}


function take_err7((function (optional named int $x): void) $_): void {}
function f_err7(named int $x): void {}
function test_err7(): void {
  take_err7(f_err7<>); // ERROR: Named parameter x is required but expected to be optional
}

function take_err8((function (named int $x): void) $_): void {}
function f_err8(named string $x): void {}
function test_err8(): void {
  take_err8(f_err8<>); // ERROR: Named parameter x is required but expected to be optional
}
