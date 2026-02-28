<?hh
<<file:__EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function take_err1((function (named int $x, named string $y): void) $_): void {}
function test_err1(): void {
  take_err1((named int $x) ==> {}); // ERROR: Missing required named parameter: y
}


function take_err2((function (named int $x): void) $_): void {}
function test_err2(): void {
  take_err2((named int $x, named string $y) ==> {}); // ERROR: Unexpected named parameter: y
}


function take_err3((function (named int $x): void) $_): void {}
function test_err3(): void {
  take_err3((named int $wrong) ==> {}); // ERROR: Missing x, Unexpected wrong
}


function take_err4((function (named int $a, named string $b): void) $_): void {}
function test_err4(): void {
  take_err4((named int $a, named string $wrong) ==> {}); // ERROR: Missing b, Unexpected wrong
}


function take_err5((function (named int $x, named string $y): void) $_): void {}
function test_err5(): void {
  take_err5((named int $p, named string $q) ==> {}); // ERROR: Missing x and y, Unexpected p and q
}


function take_err6((function (named int $a, named string $b, named bool $c): void) $_): void {}
function test_err6(): void {
  take_err6((named int $a, named float $d) ==> {}); // ERROR: Missing b and c, Unexpected d
}


function take_err7((function (optional named int $x): void) $_): void {}
function test_err7(): void {
  take_err7((named int $x) ==> {}); // ERROR: Named parameter x is required but expected to be optional
}

function take_err8((function (named int $x): void) $_): void {}
function test_err8(): void {
  take_err8((named string $x) ==> {}); // ERROR: type mismatch
}
