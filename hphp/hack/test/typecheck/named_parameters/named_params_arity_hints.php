<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

function optional_named_first(named int $n = 0, int $x): void {}
function optional_named_last(int $x, named int $n = 0): void {}
function optional_mixed(named int $n = 0, int $x, int $y = 0): void {}

function take_optional_named(
  (function(optional named int $n, int): void) $f,
): void {}

function take_optional_mixed(
  (function(optional named int $n, int, optional int): void) $f,
): void {}

function clean_calls(): void {
  optional_named_first(1);
  optional_named_first(n=2, 1);
  optional_named_last(1, n=2);
  optional_mixed(1);
  optional_mixed(1, 2, n=3);
}

function clean_lambdas(): void {
  take_optional_named((named int $n = 0, int $x) ==> {});
  take_optional_named((int $x, named int $n = 0) ==> {});
  take_optional_mixed((named int $n = 0, int $x, int $y = 0) ==> {});
  take_optional_mixed((int $x, int $y = 0, named int $n = 0) ==> {});
}
