<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file: __EnableUnstableFeatures('named_parameters')>>

// A variadic named parameter on a definition need not have a name.
function take_unnamed(int $x, named arraykey...): void {}

// A positional variadic may be followed by a variadic named parameter.
function take_both(int $x, string ...$rest, named arraykey...): void {}

abstract class C {
  abstract public function m(named bool...): void;

  public static function s(int $a, named string...): void {}
}

interface I {
  public function j(named mixed...): void;
}

function in_lambdas(): void {
  $_ = (int $x, named arraykey...) ==> $x;
  $_ = function(named int...): void {};
  $_ = async (named int...) ==> 1;
}
