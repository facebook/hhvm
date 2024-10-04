<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

// Illegal because splat is followed by another parameter
function bad1<Targs as (mixed...)>((...Targs, int) $p): void {}

// Illegal because splat is followed by a variadic
function bad2<Targs as (mixed...)>((...Targs, int...) $p): void {}

// Illegal because splat is followed by an optional
function bad3<Targs as (mixed...)>((...Targs, optional int) $p): void {}

// Illegal because splat is preceded by an optional
function bad4<Targs as (mixed...)>((optional int, ...Targs) $p): void {}

// Illegal because splat is preceded by a variadic
function bad5<Targs as (mixed...)>((int..., ...Targs) $p): void {}

interface I {
  // Illegal because splat is optional
  public function bad7<Targs as (mixed...)>((int, optional ...Targs) $p): void;
  // Illegal because splat is variadic
  public function bad8<Targs as (mixed...)>((int, ...Targs...) $p): void;
}
