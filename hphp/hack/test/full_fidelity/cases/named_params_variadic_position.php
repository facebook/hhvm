<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
<<file: __EnableUnstableFeatures('named_parameters')>>

function after_named(int $p, named string $fixed, named int...): void {}
function after_rest(int ...$xs, named string $fixed, named bool...): void {}
function after_pos(int $p, named int...): void {}

abstract class Example {
  public function __construct(int $p, named int...) {}
  abstract public function m(named int $fixed, named int...): void;
}

interface I {
  public function j(named int $fixed, named mixed...): void;
}

function lambdas(): void {
  $_ = (named int $fixed, named int...) ==> $fixed;
  $_ = function(named int $fixed, named int...): void {};
}

type FinalHint = (function(int, string..., named bool $b, named bool...): void);
type Nested = (function((function(int, named string $s, named int...): void)): void);
function ro(named readonly string $s, named string...): void {}
//
