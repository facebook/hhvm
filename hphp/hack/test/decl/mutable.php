<?hh // strict

class A {}

<<__Rx>>
function f(Rx<(function(Mutable<A>): void)> $a): void {}

<<__Rx>>
function g(Rx<(function(MaybeMutable<A>): void)> $a): void {}

<<__Rx>>
function h(Rx<(function(OwnedMutable<A>): void)> $a): void {}

<<__Rx>>
function i(Rx<(function(): OwnedMutable<A>)> $a): void {}
