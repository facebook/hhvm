<?hh // strict
class B {}
class C extends B {}

function foo1(array<B> $x): void {}
function test1(array<C> $x): void { foo1($x); }

function foo2(array<mixed, B> $x): void {}
function test2(array<int, C> $x): void { foo2($x); }

function foo3(Traversable<B> $x): void {}
function test3(array<C> $x): void { foo3($x); }

function foo4(KeyedTraversable<mixed, B> $x): void {}
function test4(array<int, C> $x): void { foo4($x); }

function foo5(Container<B> $x): void {}
function test5(array<C> $x): void { foo5($x); }

function foo6(KeyedContainer<mixed, B> $x): void {}
function test6(array<int, C> $x): void { foo6($x); }

function foo7(KeyedContainer<mixed, B> $x): void {}
function test7(array<int, C> $x): void { foo7($x); }
