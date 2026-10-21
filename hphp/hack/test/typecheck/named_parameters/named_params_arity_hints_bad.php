<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

function optional_named_first(named int $n = 0, int $x): void {}
function optional_named_last(int $x, named int $n = 0): void {}
function optional_mixed(named int $n = 0, int $x, int $y = 0): void {}

function take_optional_named(
  (function(optional named int $n, int): void) $f,
): void {}

function take_optional_positional(
  (function(int, optional int): void) $f,
): void {}

function take_optional_mixed(
  (function(optional named int $n, int, optional int): void) $f,
): void {}

function take_required_mixed(
  (function(optional named int $n, int, int): void) $f,
): void {}

function rejected_calls(): void {
  optional_named_first(1, 2);
  optional_named_last(1, 2);
  optional_mixed(1, 2, 3);
  optional_mixed(1, 2, 3, n=4);
}

function rejected_lambdas(): void {
  take_optional_named((named int $n = 0, int $x, int $extra) ==> {});
  take_optional_named((int $x, int $extra, named int $n = 0) ==> {});
  take_optional_positional((int $x, int $y) ==> {});
  take_optional_positional((int $x) ==> {});
  take_optional_mixed((named int $n = 0, int $x, int $y) ==> {});
  take_optional_mixed((named int $n = 0, int $x) ==> {});
  take_required_mixed((named int $n = 0, int $x) ==> {});
}
//
