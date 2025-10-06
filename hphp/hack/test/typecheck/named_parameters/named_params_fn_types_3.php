<?hh
<<file:__EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

// Exact match - same named parameter
function take1((function (named int $x): void) $_): void {}
function test1(): void {
  take1((named int $x) ==> {}); // OK
}

// Multiple named parameters - all match
function take2((function (named int $a, named string $b): void) $_): void {}
function test2(): void {
  take2((named int $a, named string $b) ==> {}); // OK
}

// Optional named parameter matches (using default value)
function take3((function (optional named int $x): void) $_): void {}
function test3(): void {
  take3((named int $x = 42) ==> {}); // OK - optional matches optional
}

// Mix of named and non-named parameters
function take4((function (int, named int $x): void) $_): void {}
function test4(): void {
  take4((int $a, named int $x) ==> {}); // OK
}

// Multiple named with some matching
function take5((function (named int $x, named string $y): void) $_): void {}
function test5(): void {
  take5((named int $x, named string $y) ==> {}); // OK
}
