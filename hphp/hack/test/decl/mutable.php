<?hh // strict

class A {}

<<__Pure>>
function f(Pure<(function(Mutable<A>): void)> $a): void {}

<<__Pure>>
function g(Pure<(function(MaybeMutable<A>): void)> $a): void {}

<<__Pure>>
function h(Pure<(function(OwnedMutable<A>): void)> $a): void {}

<<__Pure>>
function i(Pure<(function(): OwnedMutable<A>)> $a): void {}
