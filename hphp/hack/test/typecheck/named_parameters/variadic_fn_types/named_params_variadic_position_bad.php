<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
<<file: __EnableUnstableFeatures('named_parameters')>>

function before_named(named int..., named string $fixed): void {}
function before_positional(named int..., int $p): void {}
function before_rest(int $p, named int..., string ...$xs): void {}
function positional_rest(int ...$xs, named bool $flag, int $p): void {}

function call_before_named(): void {
  before_named(fixed = 'ok', extra = 1);
}

abstract class Base {
  abstract public function m(named int..., named string $fixed): void;
}

final class Child extends Base {
  public function __construct(named int..., int $p) {}
  public function m(named int..., named string $fixed): void {}
}

interface HasNonFinal {
  public function j(named mixed..., named string $fixed): void;
}

function lambdas(): void {
  $short = (named int..., named string $fixed) ==> $fixed;
  $long = function(named int..., named string $fixed): void {};
  $short(fixed = 'ok', extra = 1);
  $long(fixed = 'ok', extra = 1);
}

type BeforeNamed = (function(named int..., named string $fixed): void);
type BeforePositional = (function(named int..., int): void);
type BeforeRest = (function(named int..., string...): void);
type PositionalRest = (function(int..., named bool $flag, int): void);
type Nested = (function((function(named int..., int): void)): void);

function ro(named string..., named readonly string $fixed): void {}
function call_ro(readonly string $s): void {
  ro(fixed = $s, extra = 'ok');
}

function show(): void {
  hh_show(before_named<>);
}

function contextual((function(int, string, named bool $flag): void) $f): void {}
function wrong_flag(): void {
  contextual((...$args, named int $flag) ==> {});
}
//
